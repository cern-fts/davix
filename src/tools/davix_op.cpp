/*
 * This File is part of Davix, The IO library for HTTP based protocols
 * Copyright (C) CERN 2013
 * Author: Kwong Tat Cheung <kwong.tat.cheung@cern.ch>
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

#include "davix_op.hpp"
#include "davix_taskqueue.hpp"
#include <utils/davix_logger_internal.hpp>
#include <sstream>
#include <xml/s3deleteparser.hpp>
#include <xml/davdeletexmlparser.hpp>


namespace Davix{

//-------------------------------------------------
//----------------------DavixOp--------------------
//-------------------------------------------------
DavixOp::DavixOp(const Tool::OptParams& opts, std::string target_url, std::string destination_url, Context& c) :
    _target_url(target_url),
    _destination_url(destination_url),
    _opts(opts),
    _c(c),
    _scope()
{
}

DavixOp::~DavixOp(){}

std::string DavixOp::getTargetUrl(){
    return _target_url;
}

std::string DavixOp::getDestinationUrl(){
    return _destination_url;
}

std::string DavixOp::getOpType(){
    return opType;
}


//-------------------------------------------------
//----------------------GetOp----------------------
//-------------------------------------------------
GetOp::GetOp(const Tool::OptParams& opts, std::string target_url, std::string destination_url, Context& c) :
    DavixOp(opts, target_url, destination_url, c)
{
    opType = "GET";
    _scope = "Davix::DavixOp::GetOp";
}

GetOp::~GetOp(){}

int GetOp::executeOp(){
    int ret = -1;
    int fd = -1;
    DavixError* tmp_err=NULL;
    DavFile f(_c, _target_url);

    if((fd = getOutFd())> 0){
        DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CORE, "{} executing op on ", _scope, _target_url);
        ret = f.getToFd(&_opts.params, fd, &tmp_err);

        //if getToFd failed, remove the just created blank local file
        if(tmp_err){
            std::cerr << std::endl << _scope << " Failed to GET " << _target_url << std::endl;
            Tool::errorPrint(&tmp_err);
            remove(_destination_url.c_str());
        }
        close(fd);
    }
    return ret;
}

int GetOp::getOutFd(){
    DavixError* tmp_err=NULL;
    int fd = -1;
    if(_destination_url.empty() == false){
        // for S3 we have to split the key into tokens and create a dir for each token
        if (_opts.params.getProtocol() == RequestProtocol::AwsS3){
            if((Tool::mkdirP(_destination_url, true)) < 0 ){
                std::cerr << std::endl << _scope << " Failed to create local directory for " << _destination_url << std::endl;
                return -1;
            }
        }
        if((fd = open(_destination_url.c_str(), O_WRONLY | O_TRUNC | O_CREAT, 0777)) <0  ){
            davix_errno_to_davix_error(errno, _scope, std::string("while opening/creating ").append(_destination_url), &tmp_err);
            if(tmp_err){
                Tool::errorPrint(&tmp_err);
            }
            return -1;
        }

    }else{
        fd = dup(STDOUT_FILENO);
    }
    return fd;
}


//-------------------------------------------------
//----------------------PutOp----------------------
//-------------------------------------------------
PutOp::PutOp(const Tool::OptParams& opts, std::string target_url, std::string destination_url, dav_size_t file_size, Context& c) :
    DavixOp(opts, target_url, destination_url, c)
{
    opType = "PUT";
    _scope = "Davix::DavixOp::PutOp";
    _file_size = file_size;

}

PutOp::~PutOp(){}

int PutOp::executeOp(){
    DavixError* tmp_err=NULL;
    int fd = -1;

    if( (fd = getInFd(&tmp_err)) < 0){
        if(tmp_err)
            Tool::errorPrint(&tmp_err);
        return -1;
    }

    TRY_DAVIX{
        DavFile f(_c, _destination_url);
        DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CORE, "{} executing op on ", _scope, _destination_url);
        f.put(&_opts.params, fd, _file_size);
        close(fd);
        return 0;
    }CATCH_DAVIX(&tmp_err);

    if(fd != -1)
        close(fd);

    if(tmp_err->getStatus() == StatusCode::FileExist){
        std::cerr << std::endl << _scope << " " << _destination_url << " already exists, continuing..." << std::endl;
    }
    else
        std::cerr << std::endl << _scope << " Failed to PUT " << _target_url << std::endl;

    Tool::errorPrint(&tmp_err);
    return -1;
}

int PutOp::getInFd(DavixError** err){
    int fd = -1;
    if(_target_url.empty() == false){
        if((fd = open(_target_url.c_str(), O_RDONLY)) <0  ){
            davix_errno_to_davix_error(errno, _scope, std::string("for source file ").append(_target_url), err);
            return -1;
        }
    }else{
        fd = dup(STDOUT_FILENO);
    }
    return fd;

}


//-------------------------------------------------
//--------------------DeleteOp---------------------
//-------------------------------------------------
DeleteOp::DeleteOp(const Tool::OptParams& opts, std::string destination_url, Context& c, std::string buf) :
    DavixOp(opts, "", destination_url, c)
{
    opType = "DELETE";
    _scope = "Davix::DavixOp::DeleteOp";
    _buf = buf;
}

DeleteOp::~DeleteOp(){}

int DeleteOp::executeOp(){
    DavixError* tmp_err=NULL;

    DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CORE, "{} executing op on ", _scope, _destination_url);

    if(_opts.params.getProtocol() == RequestProtocol::AwsS3){
        _destination_url += "/?delete";
        PostRequest req(_c, _destination_url, &tmp_err);

        if(tmp_err){
            Tool::errorPrint(&tmp_err);
            return -1;
        }

        req.setParameters(_opts.params);

        std::ostringstream ss;
        ss << _buf.size();

        // calculate md5 of body and set header fields, these are required for S3 multi-objects delete
        std::string md5;
        S3::calculateMD5(_buf, md5);

        req.addHeaderField("Content-MD5", md5);
        req.addHeaderField("Content-Length", ss.str());

        req.setRequestBody(_buf);

        req.executeRequest(&tmp_err);

        if(tmp_err){
            Tool::errorPrint(&tmp_err);
            return -1;
        }

        // check response code
        int code = req.getRequestCode();

        if(!httpcodeIsValid(code)){
            httpcodeToDavixError(req.getRequestCode(), davix_scope_http_request(), "during S3 multi-objects delete operation", &tmp_err);
            if(tmp_err){
                Tool::errorPrint(&tmp_err);
                return -1;
            }
        }

        std::vector<char> body = req.getAnswerContentVec();

        TRY_DAVIX{
            parse_deletion_result(code, Uri(_destination_url), _scope, body);
        }CATCH_DAVIX(&tmp_err);

        if(tmp_err){
            Tool::errorPrint(&tmp_err);
            return -1;
        }
    }
    else{
        // cases other than s3, not implenmented for now. WebDAV delete collection already works without the -r switch
        return -1;
    }

    return 0;
}

void DeleteOp::parse_deletion_result(int code, const Uri & u, const std::string & scope, const std::vector<char> & body){
    switch(code){
        case 200:{
            // if s3 && scope was davix_scope_rm_str() && batch delete, parse body
            S3DeleteParser parser;
            parser.parseChunk(&(body[0]), body.size());

            // check if any of the delete request entry is flagged as error, if so, just print them for now
            if( parser.getDeleteStatus().size() > 0){
                for(unsigned int i=0; i < parser.getDeleteStatus().size(); ++i){
                    if(parser.getDeleteStatus().at(i).error){
                        std::ostringstream ss;
                        ss << "Error: " << parser.getDeleteStatus().at(i).error_code <<
                            " -> " << parser.getDeleteStatus().at(i).message <<
                            " encountered while atempting to delete " <<
                            parser.getDeleteStatus().at(i).filename;

                        std::cerr << std::endl << ss.str() << std::endl;
                    }
                    return;
                }
                // if no properties, status were filtered because invalid
                httpcodeToDavixException(404, scope);
                return;
            }
        }
        case 201:
        case 202:
        case 204:{
                return;
        }
        case 207:{
            // parse webdav
            DavDeleteXMLParser parser;
            parser.parseChunk(&(body[0]), body.size());
            if( parser.getProperties().size() > 0){
                for(unsigned int i=0; i < parser.getProperties().size(); ++i){
                   const int sub_code = parser.getProperties().at(i).req_status;
                   std::ostringstream ss;

                   ss << "occurred during deletion request for " << parser.getProperties().at(i).filename;

                   if(httpcodeIsValid(sub_code) == false){
                       httpcodeToDavixException(sub_code, scope, ss.str());
                   }
                }

               return;
            }
            // if no properties, properties were filtered because invalid
            httpcodeToDavixException(404, scope);
            break;
        }
    }
    std::ostringstream ss;
    ss << " with url " << u.getString();
    httpcodeToDavixException(code, scope, ss.str());
}


//-------------------------------------------------
//---------------------ListOp----------------------
//-------------------------------------------------
ListOp::ListOp(const Tool::OptParams& opts, std::string target_url, Context& c, DavixTaskQueue* listing_tq, FILE* filestream, pthread_mutex_t& output_mutex) :
    DavixOp(opts, target_url, "NULL", c),
    _listing_tq(listing_tq),
    _filestream(filestream),
    _output_mutex(output_mutex)
{
    opType = "LIST";
    _scope = "Davix::DavixOp::ListOp";
}

ListOp::~ListOp(){}

void ListOp::display_file_entry(const std::string & filename, const Tool::OptParams & opts, FILE* filestream){
    (void) opts;
    fputs(filename.c_str(), filestream);
    fputs("\n",filestream);
}

void ListOp::display_long_file_entry(const std::string & filename,  struct stat* st, const Tool::OptParams & opts, FILE* filestream){
    (void) opts;
    std::ostringstream ss;
    ss << Tool::string_from_mode(st->st_mode) << " ";
    ss << Tool::string_from_size_t(static_cast<size_t>(st->st_nlink),4) << " ";
    ss << Tool::string_from_size_t(st->st_size, 9) << " ";
    ss << Tool::string_from_ptime(st->st_mtime) << " ";
    ss << filename << "\n";

    fputs(ss.str().c_str(), filestream);
}

struct DirEntry {
    std::string fullURL;
    std::string path;
    struct stat st;
    DirEntry(std::string _fullURL, std::string _path, struct stat _st)
        : fullURL(_fullURL), path(_path), st(_st) {}
};

struct FileEntry {
    std::string path;
    struct stat st;
    FileEntry(std::string _path, struct stat _st)
        : path(_path), st(_st) {}
};

int ListOp::executeOp(){
    DAVIX_DIR* fd = NULL;
    DavPosix pos(&_c);
    DavixError* tmp_err=NULL;
    std::string outputPath;
    struct stat st;
    struct dirent* d;
    unsigned long entry_counter = 0;
    std::string last_success_entry;

    std::deque< struct DirEntry > dirQueue;
    std::deque< struct FileEntry > fileQueue;

    // set up first entry
    if(_target_url.empty()){
        DavixError::setupError(&tmp_err, "Davix::ListOp", StatusCode::InvalidArgument, " target URL is empty.");
    }

    if( (fd = pos.opendirpp(&_opts.params, _target_url, &tmp_err)) == NULL){
        Tool::errorPrint(&tmp_err);
        return -1;
    }

    Uri tmp(_target_url);
    std::string fullpath = tmp.getPath();
    if(fullpath[0] == '/') fullpath.erase(0,1);

    while( ((d = pos.readdirpp(fd, &st, &tmp_err)) != NULL)){    // if one entry inside a directory fails, the loop exits, the other entires are not processed

        last_success_entry = Uri::join(_target_url, d->d_name);
        // for each entry, see if it's a directory, if yes, push to dirQueue for further processing
        if(st.st_mode & S_IFDIR){
            DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CORE, "Directory entry found, pushing {}/ to dirQueue", _target_url+d->d_name);
            dirQueue.push_back(DirEntry(Uri::join(_target_url, d->d_name)+"/", Uri::join(fullpath, d->d_name), st));
        }
        else{
            fileQueue.push_back(FileEntry(Uri::join(fullpath, d->d_name), st));
        }

    } // while readdirpp


    if(tmp_err){
        Tool::errorPrint(&tmp_err);
        std::cerr << std::endl << "Error occurred during listing  " << _target_url << " Number of entries processed in current directory: " << entry_counter << ". Continuing..."<< std::endl;
        std::cerr << std::endl << "Last successful entry is " << last_success_entry << std::endl;
    }

    entry_counter = 0;
    pos.closedirpp(fd, NULL);

    {
        // lock this part so the output is sync'ed
        // this should also ensure all the child ops are ordered in the taskqueue
        DavixMutex mutex(_output_mutex);

        for(unsigned int i=0; i < dirQueue.size(); ++i){
            //push listing op to task queue
            DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CORE, "Adding item to (listing) work queue, target is {}.", dirQueue[i].fullURL);
            ListOp* l_op = new ListOp(_opts, dirQueue[i].fullURL, _c, _listing_tq, _filestream, _output_mutex);
            _listing_tq->pushOp(l_op);

            if(_opts.pres_flag & LONG_LISTING_FLAG){
                display_long_file_entry(dirQueue[i].path, &(dirQueue[i].st), _opts, _filestream);
            }else{
                display_file_entry(dirQueue[i].path, _opts, _filestream);
            }
        }
        for(unsigned int i=0; i < fileQueue.size(); ++i){
            if(_opts.pres_flag & LONG_LISTING_FLAG){
                display_long_file_entry(fileQueue[i].path, &(fileQueue[i].st), _opts, _filestream);
            }else{
                display_file_entry(fileQueue[i].path, _opts, _filestream);
            }
        }

    }

    if(tmp_err){
        Tool::errorPrint(&tmp_err);
        return -1;
    }
    return 0;
}


//-------------------------------------------------
//---------------------ListppOp----------------------
//-------------------------------------------------
ListppOp::ListppOp(const Tool::OptParams& opts, std::string target_url, std::string destination_url, Context& c, DavixTaskQueue* tq, DavixTaskQueue* listing_tq) :
    DavixOp(opts, target_url, destination_url, c)
{
    opType = "LISTPP";
    _scope = "Davix::DavixOp::ListppOp";
    _tq = tq;
    _listing_tq = listing_tq;
}

ListppOp::~ListppOp(){}

int ListppOp::executeOp(){
    DAVIX_DIR* fd = NULL;
    DavPosix pos(&_c);
    DavixError* tmp_err=NULL;
    std::string outputPath;
    struct stat st;
    struct dirent* d;
    unsigned long entry_counter=0;
    std::string last_success_entry;

    std::deque<std::pair<std::string,std::string> > dirQueue;
    std::deque<std::pair<std::string,std::string> > opQueue;

    // set up first entry
    if(!_target_url.empty() && !_destination_url.empty()){
        dirQueue.push_back(std::make_pair(_target_url, _destination_url));
    }
    else
        DavixError::setupError(&tmp_err, "Davix::ListppOp", StatusCode::InvalidArgument, " target or destination URL is empty.");

    if( (fd = pos.opendirpp(&_opts.params, dirQueue.front().first, &tmp_err)) == NULL){
        Tool::errorPrint(&tmp_err);

        return -1;
    }

    // dir opened successfully, create local dir
    Tool::mkdirP(dirQueue.front().second, false);

    while( ((d = pos.readdirpp(fd, &st, &tmp_err)) != NULL)){    // if one entry inside a directory fails, the loop exits, the other entires are not processed

        last_success_entry = Uri::join(dirQueue.front().first, d->d_name);
        // for each entry, see if it's a directory, if yes, push to dirQueue for further processing
        if(st.st_mode & S_IFDIR){
            DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CORE, "Directory entry found, pushing {}/ to dirQueue", dirQueue.front().first+d->d_name);
            dirQueue.push_back(std::make_pair(Uri::join(dirQueue.front().first, d->d_name)+"/",Uri::join(dirQueue.front().second, d->d_name)));
        }
        else if(!(st.st_mode & S_IFDIR)){
            //if we spend too long in here, server will likely close the connection mid-readdirpp, need to get all the entries quickly before processing them
            opQueue.push_back(std::make_pair(Uri::join(dirQueue.front().first, d->d_name), Uri::join(dirQueue.front().second, d->d_name)));
            entry_counter++;
        }
    } // while readdirpp

    if(tmp_err){
        Tool::errorPrint(&tmp_err);
        std::cerr << std::endl << "Error occurred during listing  " << dirQueue.front().first << " Number of entries processed in current directory: " << entry_counter << ". Continuing..."<< std::endl;
        std::cerr << std::endl << "Last successful entry is " << last_success_entry << std::endl;
    }

    pos.closedirpp(fd, NULL);
    dirQueue.pop_front();

    for(unsigned int i=0; i < dirQueue.size(); ++i){
        //push listing op to task queue
        DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CORE, "Adding item to (listing) work queue, target is {} and destination is {}.", dirQueue[i].first, dirQueue[i].second);
        ListppOp* l_op = new ListppOp(_opts, (dirQueue[i].first), (dirQueue[i].second), _c, _tq, _listing_tq);
        _listing_tq->pushOp(l_op);
    }

    for(unsigned int i=0; i < opQueue.size(); ++i){
        //push op to task queue
        DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CORE, "Adding item to (get) work queue, target is {} and destination is {}.", opQueue[i].first, opQueue[i].second);

        GetOp* op = new GetOp(_opts, (opQueue[i].first), (opQueue[i].second), _c);
        _tq->pushOp(op);
    }

    if(tmp_err){
        Tool::errorPrint(&tmp_err);
        return -1;
    }
    return 0;
}


} // namespace Davix
