/*
 * This File is part of Davix, The IO library for HTTP based protocols
 * Copyright (C) CERN 2013
 * Author: Adrien Devresse <adrien.devresse@cern.ch>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
*/

#include <davix_internal.hpp>
#include "httpiovec.hpp"
#include <utils/davix_logger_internal.hpp>
#include <utils/stringutils.hpp>
#include "libs/IntervalTree.h"

#include <map>

#define DBG(message) std::cerr << __FILE__ << ":" << __LINE__ << " -- " << #message << " = " << message << std::endl;
using namespace StrUtil;

// remove trailing crlf
template<class InputIterator>
  dav_size_t trim_crlf (InputIterator first, InputIterator last, dav_size_t s)
{
  dav_size_t ret = s;
  while (--last >= first) {
      if(*last == '\n' || *last == '\r'){
          *last = '\0';
          ret--;
      }else{
          break;
      }
  }
  return ret;
}



namespace Davix{

const std::string HttpIoVec_scope(){
    return "Davix::HttpIOVecOps";
}


void HttpIoVecSetupErrorMultiPart(DavixError** err){
    DavixError::setupError(err, HttpIoVec_scope(), StatusCode::InvalidServerResponse, "Invalid Multi-Part HTTP response");
}

void HttpIoVecSetupErrorMultiPartTooLong(DavixError** err){
    DavixError::setupError(err, HttpIoVec_scope(), StatusCode::InvalidServerResponse, "Invalid Multi-Part HTTP, Multi-part header too long");
}

void HttpIoVecSetupErrorMultiPartBoundary(const std::string & boundary, DavixError** err){
    DavixError::setupError(err, HttpIoVec_scope(), StatusCode::InvalidServerResponse, std::string("Invalid boundary for multipart http response :").append(boundary));
}

void HttpIoVecSetupErrorMultiPartSize( DavixError** err, dav_off_t req_offset, dav_size_t req_size, dav_off_t ans_offset, dav_size_t ans_size){
    std::ostringstream ss;
    ss << "Invalid server answer for multi part, request offset:"<< req_offset <<" size:"<< req_size << ", answer offset:"<< ans_offset<< " size:"<< ans_size;
    DavixError::setupError(err, HttpIoVec_scope(), StatusCode::InvalidServerResponse, ss.str());
}

inline char* header_delimiter(char* buffer, dav_size_t len){
    char* p = std::find(buffer, buffer + len, ':');
    return (p < buffer + len)?p:NULL;
}

// Vector operation option provider
/*int davIOVecProvider(const DavIOVecInput *input_vec, dav_ssize_t & counter, dav_ssize_t number, dav_off_t & begin, dav_off_t & end){
    if(counter < number){
        begin = input_vec[counter].diov_offset;
        end = std::max<dav_off_t>(begin + input_vec[counter].diov_size -1, begin);
        return ++counter;
    }
    return -1;
}*/

int davIOVecProvider(const SortedRanges ranges, dav_size_t & counter, dav_off_t & begin, dav_off_t & end) {
    if(counter < ranges.size()) {
        begin = ranges[counter].first;
        end = ranges[counter].second;
        return ++counter;
    }
    return -1;
}


// do a multi-range on selected ranges
MultirangeResult HttpIOVecOps::performMultirange(IOChainContext & iocontext,
                                                 const IntervalTree<ElemChunk> &tree,
                                                 const SortedRanges & ranges) {

    DavixError * tmp_err=NULL;
    dav_ssize_t tmp_ret=-1, ret = 0;
    ptrdiff_t p_diff=0;
    dav_size_t counter = 0;
    MultirangeResult::OperationResult opresult = MultirangeResult::SUCCESS;

    // calculate total bytes to be read (approximate, since ranges could overlap)
    dav_ssize_t bytes_to_read = 0;
    for(dav_size_t i = 0; i < ranges.size(); i++) {
        bytes_to_read += (ranges[i].second - ranges[i].first + 1);
    }

    std::function<int (dav_off_t &, dav_off_t &)> offsetProvider = std::bind(&davIOVecProvider, ranges, std::ref(counter), std::placeholders::_1, std::placeholders::_2);

    // header line need to be inferior to 8K on Apache2 / ngnix
    // in Addition, some S3 implementation limit the total header size to 4k....
    // 3900 bytes maximum for the range seems to be a ood compromise
    std::vector< std::pair<dav_size_t, std::string> > vecRanges = generateRangeHeaders(3900, offsetProvider);

    DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CHAIN, " -> getPartialVec operation for {} vectors", ranges.size());

    for(std::vector< std::pair<dav_size_t, std::string> >::iterator it = vecRanges.begin(); it < vecRanges.end(); ++it){
        DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CHAIN, " -> getPartialVec request for {} chunks", it->first);

        if(it->first == 1){ // one chunk only : no need of multi part
            ret += singleRangeRequest(iocontext, tree, ranges[p_diff].first, ranges[p_diff].second - ranges[p_diff].first + 1);
            p_diff += 1;
        }else{
            GetRequest req (iocontext._context, iocontext._uri, &tmp_err);
            if(tmp_err == NULL){
                RequestParams request_params(iocontext._reqparams);
                req.setParameters(request_params);
                req.addHeaderField(req_header_byte_range, it->second);

                if( req.beginRequest(&tmp_err) == 0){
                    const int retcode = req.getRequestCode();

                    // looks like the server supports multi-range requests.. yay
                    if(retcode == 206) {
                        ret = parseMultipartRequest(req, tree, &tmp_err);

                        // could not parse multipart response - server's broken?
                        // known to happen with ceph - return code is 206, but only
                        // returns the first range
                        if(ret == -1) {
                            opresult = MultirangeResult::NOMULTIRANGE;
                            req.endRequest(&tmp_err);
                            break;
                        }
                    }
                    // no multi-range.. bad server, bad
                    else if(retcode == 200) {
                        DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CHAIN, "Multi-range request resulted in getting the whole file.");
                        // we have two options: read the entire file or abort current
                        // request and start a multi-range simulation

                        // if this is a huge file, reading the entire contents is
                        // definitely not an option
                        if(req.getAnswerSize() > 1000000 && req.getAnswerSize() > 2*bytes_to_read) {
                            DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CHAIN, "File is too large; will not waste bandwidth, bailing out");
                            opresult = MultirangeResult::NOMULTIRANGE;
                            req.endRequest(&tmp_err);
                        }
                        else {
                            DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CHAIN, "Simulating multi-part response from the contents of the entire file");
                            opresult = MultirangeResult::SUCCESS_BUT_NO_MULTIRANGE;
                            ret = simulateMultiPartRequest(req, tree, &tmp_err);
                        }
                        break;
                    }
                    else if(retcode == 416) {
                      ret = 0;
                      DavixError::clearError(&tmp_err);
                    }
                    else {
                        httpcodeToDavixError(req.getRequestCode(),davix_scope_http_request(),", ", &tmp_err);
                        ret = -1;
                        break;
                    }

                    p_diff += it->first;
                    ret += tmp_ret;
                } else {
                   ret = -1;
                   break;
                }
            }
        }
    }

    DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CHAIN, " <- getPartialVec operation for {} vectors", ranges.size());
    checkDavixError(&tmp_err);
    return MultirangeResult(opresult, ret);
}

/* fire off a single, one-range request */
dav_ssize_t HttpIOVecOps::singleRangeRequest(IOChainContext & iocontext,
                                     const DavIOVecInput * input,
                                     DavIOVecOuput * output) {
    dav_ssize_t size = _start->pread(iocontext,
                                     input->diov_buffer,
                                     input->diov_size,
                                     input->diov_offset);

    output->diov_size = size;
    output->diov_buffer = input->diov_buffer;
    return size;
}

// merge user-provided ranges, add them to intervals
typedef std::multimap<dav_off_t, dav_size_t> MergedRanges;
static SortedRanges partialMerging(const IntervalTree<ElemChunk> &tree, const dav_size_t mergedist) {
    MergedRanges merged;

    std::vector<Interval<ElemChunk> > allranges_unsorted;
    tree.findContained(0, std::numeric_limits<dav_size_t>::max(), allranges_unsorted);

    MergedRanges allranges;
    for(std::vector<Interval<ElemChunk> >::iterator it = allranges_unsorted.begin(); it != allranges_unsorted.end(); it++)
        allranges.insert(std::make_pair(it->start, it->stop));

    dav_off_t offset = allranges.begin()->first;
    dav_off_t end = allranges.begin()->second;

    for(MergedRanges::iterator it = allranges.begin(); it != allranges.end(); it++) {
        if(end + (dav_off_t) mergedist >= it->first) {
            end = it->second;
        }
        else {
            merged.insert(std::make_pair(offset, end));
            offset = it->first;
            end = it->second;
        }
    }
    merged.insert(std::make_pair(offset, end));

    SortedRanges output;
    for(MergedRanges::iterator it = merged.begin(); it != merged.end(); it++)
        output.push_back(*it);

    return output;
}

static IntervalTree<ElemChunk> buildIntervalTree(const DavIOVecInput *in, DavIOVecOuput *out, const dav_size_t count_vec) {
    std::vector<Interval<ElemChunk> > intervals;

    for(dav_size_t i = 0; i < count_vec; i++) {
        dav_off_t start = in[i].diov_offset;
        dav_off_t end = start + in[i].diov_size - 1;
        ElemChunk elem(in+i, out+i);

        intervals.push_back(Interval<ElemChunk>(start, end, elem));
    }
    return IntervalTree<ElemChunk>(intervals);
}

typedef struct thread_data {
    HttpIOVecOps *ptr;
    int thread_no;
    const SortedRanges *ranges;
    const IntervalTree<ElemChunk> *tree;
    IOChainContext *iocontext;
    dav_size_t start, end;

    dav_ssize_t size;
    std::exception_ptr exc;
} thdata;

void* parallelSingleRange(void *args) {

    thdata *data = (thdata*) args;
    const SortedRanges & ranges = *data->ranges;

    data->size = 0;
    for(dav_size_t i = data->start; i < data->end; i++) {
        try {
          data->size += data->ptr->singleRangeRequest(*data->iocontext, *data->tree,
                                                      ranges[i].first,
                                                      ranges[i].second - ranges[i].first + 1);
        }
        catch(const Davix::DavixException &err) {
          data->exc = std::current_exception();
          return NULL;
        }
    }

    return NULL;
}


dav_ssize_t HttpIOVecOps::simulateMultirange(IOChainContext & iocontext,
                                     const IntervalTree<ElemChunk> & tree,
                                     const SortedRanges & ranges,
                                     const uint nconnections) {
    DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CHAIN, "Simulating a multi-range request with {} vectors", ranges.size());
    dav_ssize_t size = 0;

    uint num_threads = nconnections;
    if(num_threads > ranges.size()) {
        num_threads = ranges.size();
    }
    uint queries_per_thread = ranges.size() / num_threads;

    pthread_t threads[num_threads];
    thdata data[num_threads];
    for(uint i = 0; i < num_threads; i++) {
        data[i].ptr = this;
        data[i].thread_no = i;
        data[i].ranges = &ranges;
        data[i].tree = &tree;
        data[i].iocontext = &iocontext;

        data[i].start = i*queries_per_thread;
        data[i].end = data[i].start + queries_per_thread;

        if(i == num_threads - 1) {
            data[i].end = ranges.size();
        }

        pthread_create(&threads[i], NULL, parallelSingleRange, (void*) &data[i]);
    }

    for(uint i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
        size += data[i].size;
    }

    for(uint i = 0; i < num_threads; i++) {
        if(data[i].exc != std::exception_ptr()) {
            std::rethrow_exception(data[i].exc);
        }
    }

    return size;
}

dav_ssize_t HttpIOVecOps::preadVec(IOChainContext & iocontext, const DavIOVecInput * input_vec,
                          DavIOVecOuput * output_vec,
                          const dav_size_t count_vec){
    if(count_vec ==0)
        return 0;

    for(dav_size_t i = 0; i < count_vec; i++) {
      output_vec[i].diov_size = 0;
    }

    // size of merge window
    dav_size_t mergewindow = 2000;
    if(iocontext._uri.fragmentParamExists("mergewindow")) {
        mergewindow = atoi(iocontext._uri.getFragmentParam("mergewindow").c_str());
        DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CHAIN, "Setting mergewindow to {}", mergewindow);
    }

    // number of parallel connections in case of a simulation
    uint nconnections = 3;
    if(iocontext._uri.fragmentParamExists("nconnections")) {
        nconnections = atoi(iocontext._uri.getFragmentParam("nconnections").c_str());
        DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CHAIN, "Setting number of desired parallel connections to {}", nconnections);
    }

    IntervalTree<ElemChunk> tree = buildIntervalTree(input_vec, output_vec, count_vec);

    // a lot of servers do not support multirange... should we even try?
    if(count_vec == 1 || iocontext._uri.getFragmentParam("multirange") == "false") {
        SortedRanges sorted = partialMerging(tree, mergewindow);
        return simulateMultirange(iocontext, tree, sorted, nconnections);
    }

    SortedRanges sorted = partialMerging(tree, mergewindow);
    MultirangeResult res = performMultirange(iocontext, tree, sorted);
    if(res.res == MultirangeResult::SUCCESS || res.res == MultirangeResult::SUCCESS_BUT_NO_MULTIRANGE) {
        return res.size_bytes;
    }
    else {
        DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CHAIN, "Multi-range request has failed, attempting to recover by using multiple single-range requests");
        sorted = partialMerging(tree, mergewindow);
        return simulateMultirange(iocontext, tree, sorted, nconnections);
    }
}

int http_extract_boundary_from_content_type(const std::string & buffer, std::string & boundary, DavixError** err){
    dav_size_t pos_bound;
    static const std::string delimiter = "\";";
    if( (pos_bound= buffer.find(ans_header_boundary_field)) != std::string::npos){
        std::vector<std::string> tokens = tokenSplit(buffer.substr(pos_bound + ans_header_boundary_field.size()), delimiter);
        if( tokens.size() >= 1
            && tokens[0].size() > 0
            && tokens[0].size() <= 70){
            DAVIX_SLOG(DAVIX_LOG_TRACE, DAVIX_LOG_CHAIN, "Multi part boundary: {}", boundary);
            std::swap(boundary,tokens[0]);
            return 0;
        }
    }
    return -1;
}


int get_multi_part_info(const HttpRequest& req, std::string & boundary, DavixError** err){
    std::string buffer;

    if( req.getAnswerHeader(ans_header_content_type, buffer) == true // has content type
           && http_extract_boundary_from_content_type(buffer, boundary, err) == 0){
          return 0;
    }
    return -1;
}

// analyze header and try to find size of the part
// return 0 -> not a content length header, return -1 : not a header or error, return 1 : success
int find_header_params(char* buffer, dav_size_t buffer_len, dav_size_t* part_size, dav_off_t* part_offset){
    static const std::string delimiter(" bytes-/\t");
    char * p = header_delimiter(buffer, buffer_len);
    if(p == NULL)
        return -1;
    std::string header_type(buffer, p - buffer);
    if( compare_ncase(ans_header_byte_range, 0, p - buffer, buffer) !=0) // check header type
        return 0;

    std::vector<std::string> tokens = tokenSplit(std::string(p+1),delimiter);     // parse header
    if(tokens.size() < 2)
        return -1;

    long chunk_size[2];
    for(int i =0; i <2;++i){
        chunk_size[i]= strtol(tokens[i].c_str(), &p, 10);
        if(chunk_size[i] == LONG_MAX || chunk_size[i] < 0 || *p != '\0'){
            errno =0;
            return -1;
        }
    }
    if(chunk_size[1] < chunk_size[0])
        return -1;

    *part_offset= chunk_size[0];
    *part_size =  chunk_size[1]-chunk_size[0]+1;
    return 1;
}

inline dav_ssize_t parse_multi_part_header_line(HttpRequest& req, char* buffer, DavixError** err){
    dav_ssize_t ret =0;
    if( (ret = req.readLine(buffer, DAVIX_READ_BLOCK_SIZE, err)) <0 ){
        return -1;
    }
    ret=  trim_crlf(buffer, buffer + ret, ret);
    return ret;
}

enum BoundaryType { NORMAL_BOUNDARY, TERMINATING_BOUNDARY, NOT_A_BOUNDARY };

static BoundaryType parseBoundary(char* buffer, const std::string & boundary) {
    dav_size_t len = strlen(buffer);

    if(len <= 3)
        return NOT_A_BOUNDARY;

    char *p = buffer;
    if(*p != '-' || *(p+1) != '-')
        return NOT_A_BOUNDARY;

    if(strncmp(buffer+2, boundary.c_str(), boundary.size()) != 0)
        return NOT_A_BOUNDARY;

    if(boundary.size()+2 == len)
        return NORMAL_BOUNDARY;

    if(boundary.size()+4 == len && *(p+len-2) == '-' && *(p+len-1) == '-')
        return TERMINATING_BOUNDARY;

    return NOT_A_BOUNDARY;
}

int  parse_multi_part_header(HttpRequest& req, const std::string & boundary, ChunkInfo & info,
                            int & n_try, DavixError** err){
    dav_ssize_t ret =0;
    char buffer[DAVIX_READ_BLOCK_SIZE+1] = {0};

    if(n_try > 100){
        HttpIoVecSetupErrorMultiPartTooLong(err);
        return -1;
    }

    if( (ret = parse_multi_part_header_line(req, buffer, err)) <0 ){
        return -1;
    }

    if(!info.bounded){
        if(ret == 0) // start with crlf
            return parse_multi_part_header(req, boundary, info, ++n_try, err);

        BoundaryType type = parseBoundary(buffer, boundary);
        if(type == NOT_A_BOUNDARY) return -1;
        if(type == TERMINATING_BOUNDARY) return -2;

        info.bounded = true;
        return parse_multi_part_header(req, boundary, info, ++n_try, err);
    }

    if( info.offset == 0 && info.size == 0){
        if( find_header_params(buffer, ret, &(info.size), &(info.offset)) < 0)
            return -1;
        return parse_multi_part_header(req, boundary, info, ++n_try, err);
    }
    if(ret == 0) // end crlf
        return 0;

    HttpIoVecSetupErrorMultiPart(err);
    return -1;
}

// copy from source to chunk
static void copyBytes(const char *source, dav_off_t offset, dav_size_t size, ElemChunk &chunk) {
    dav_off_t chunkOffset = chunk._in->diov_offset;
    dav_size_t chunkSize = chunk._in->diov_size;

    // first byte from which we'll start copying
    dav_off_t common_offset = std::max(offset, chunkOffset);

    // the length for which the two segments intersect
    dav_size_t intersect_dist = std::min(size - (common_offset-offset), chunkSize - (common_offset-chunkOffset));

    memcpy( (char*) chunk._in->diov_buffer+(common_offset-chunkOffset),
            source+(common_offset-offset), intersect_dist);

    chunk._ou->diov_buffer = chunk._in->diov_buffer;
    chunk._ou->diov_size += intersect_dist;
}

// find all matching chunks in tree and fill them
static void fillChunks(const char *source, const IntervalTree<ElemChunk> &tree, dav_off_t offset, dav_size_t size) {
    std::vector<Interval<ElemChunk> > matches;
    tree.findOverlapping(offset, offset+size-1, matches);

    for(std::vector<Interval<ElemChunk> >::iterator it = matches.begin(); it != matches.end(); it++) {
        copyBytes(source, offset, size, it->value);
    }

    if(matches.size() == 0) {
        DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CHAIN, "WARNING: Received byte-range from server does not match any in the interval tree");
    }
}

dav_ssize_t copyChunk(HttpRequest & req, const IntervalTree<ElemChunk> &tree, dav_off_t offset, dav_size_t size,
                      DavixError** err){
    DavixError* tmp_err=NULL;
    dav_ssize_t ret;
    DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CHAIN, "Davix::parseMultipartRequest::copyChunk copy {} bytes with offset {}", size, offset);

    std::vector<char> buffer;
    buffer.resize(size+1);

    ret = req.readSegment(&buffer[0], size, &tmp_err);
    if(ret != (dav_ssize_t) size || tmp_err) {
        DavixError::propagateError(err, tmp_err);
    }
    else {
        fillChunks(&buffer[0], tree, offset, size);
    }

    return ret;
}

dav_ssize_t HttpIOVecOps::singleRangeRequest(IOChainContext & iocontext,
                                             const IntervalTree<ElemChunk> & tree,
                                             dav_off_t offset, dav_size_t size) {
    std::vector<char> buffer;
    buffer.resize(size+1);

    dav_ssize_t s = _start->pread(iocontext, &buffer[0], size, offset);
    fillChunks(&buffer[0], tree, offset, s);
    return s;
}

dav_ssize_t HttpIOVecOps::parseMultipartRequest(HttpRequest & _req,
                                                const IntervalTree<ElemChunk> & tree,
                                                DavixError** err) {
    std::string boundary;
    dav_ssize_t ret = 0, tmp_ret =0;
    DAVIX_SLOG(DAVIX_LOG_TRACE, DAVIX_LOG_CHAIN, "Davix::parseMultipartRequest multi part parsing");

    if(get_multi_part_info(_req, boundary, err) != 0){
        DAVIX_SLOG(DAVIX_LOG_TRACE, DAVIX_LOG_CHAIN, "Invalid Header Content info for multi part request");
        return -1;
    }
    DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CHAIN, "Davix::parseMultipartRequest multi-part boundary {}", boundary);

    while(1) {
       DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CHAIN, "Davix::parseMultipartRequest parsing a new chunk");
       ChunkInfo infos;
       int n_try = 0;

       tmp_ret = parse_multi_part_header(_req, boundary, infos, n_try, err);
       if(tmp_ret == -2) break; // terminating boundary
       if(tmp_ret == -1) return -1; // error

       if( (tmp_ret = copyChunk(_req, tree, infos.offset, infos.size, err)) <0 )
           return -1;

       ret += tmp_ret;
       DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CHAIN, "Davix::parseMultipartRequest chunk parsed with success, next chunk..");
    }

    // finish with success, dump the remaining part of the query to end the request properly
    char buffer[255];
    while( _req.readBlock(buffer, 255, NULL) > 0);

    return ret;
}

dav_ssize_t HttpIOVecOps::simulateMultiPartRequest(HttpRequest & _req, const IntervalTree<ElemChunk> & tree, DavixError** err) {
    DAVIX_SLOG(DAVIX_LOG_TRACE, DAVIX_LOG_CHAIN, " -> Davix vec : 200 full file, simulate vec io");
    char buffer[DAVIX_READ_BLOCK_SIZE+1];
    dav_ssize_t partial_read_size = 0, total_read_size = 0;
    while( (partial_read_size = _req.readBlock(buffer, DAVIX_READ_BLOCK_SIZE, err)) >0) {
        fillChunks(buffer, tree, total_read_size, partial_read_size);
        total_read_size += partial_read_size;
    }

    return total_read_size;
}



} // Davix
