#include "parser_test.hpp"


#include <davix.hpp>
#include <fileops/httpiovec.hpp>
#include <fileops/fileutils.hpp>
#include <gtest/gtest.h>

using namespace std;
using namespace Davix;

TEST(IOVecMultiPartParser, headerParser){
    std::string header;
    dav_size_t size;
    dav_off_t offset;
    header = "Content-type: application/xml"; // random generic header

    int ret = find_header_params((char*) header.c_str(), header.size(), &size, &offset);
    ASSERT_EQ(ret,0);

    header = "big brother is wathing you";

    ret = find_header_params((char*) header.c_str(), header.size(), &size, &offset);
    ASSERT_EQ(ret,-1);


    header = "Content-Range: bytes 600-900/8000";
    ret = find_header_params((char*) header.c_str(), header.size(), &size, &offset);
    ASSERT_EQ(1,ret);
    ASSERT_EQ(301, size);
    ASSERT_EQ(600, offset);

    header = "conTent-range: bytes 600-900/8000"; // break case
    ret = find_header_params((char*) header.c_str(), header.size(), &size, &offset);
    ASSERT_EQ(1,ret);
    ASSERT_EQ(301, size);
    ASSERT_EQ(600, offset);



    header = "conTent-range: bytes 600ssss-9e00/8000"; // break case
    ret = find_header_params((char*) header.c_str(), header.size(),&size, &offset);
    ASSERT_EQ(ret,-1);

    header = "Content-Range: GalaticCreditStandard 600-900/8000";
    ret = find_header_params((char*) header.c_str(), header.size(), &size, &offset);
    ASSERT_EQ(-1,ret);

}


TEST(IOVecMultiPartParser, BoundaryExtract){
    std::string header, boundary;
    DavixError* tmp_err = NULL;
    header = " multipart/mixed; boundary=gc0p4Jq0M2Yt08jU534c0p";

    int ret = http_extract_boundary_from_content_type(header, boundary, &tmp_err);
    ASSERT_EQ(0,ret );
    ASSERT_STREQ(boundary.c_str(), "gc0p4Jq0M2Yt08jU534c0p");
    ASSERT_EQ(NULL, tmp_err);

    boundary = "";
    header = " multipart/mixed; boundary=";
    ret = http_extract_boundary_from_content_type(header, boundary, &tmp_err);
    ASSERT_EQ(-1,ret );
    ASSERT_TRUE(tmp_err != NULL);
    DavixError::clearError(&tmp_err);

    boundary = "";
    header = " multipart/mixed; boundary=\"helloworld\"";
    ret = http_extract_boundary_from_content_type(header, boundary, &tmp_err);
    ASSERT_EQ(0,ret );
    ASSERT_STREQ(boundary.c_str(), "helloworld");
    ASSERT_EQ(NULL, tmp_err);


    boundary = "";
    header = " multipart/mixed; boundary=helloworld; some trash strng";
    ret = http_extract_boundary_from_content_type(header, boundary, &tmp_err);
    ASSERT_EQ(0,ret );
    ASSERT_STREQ(boundary.c_str(), "helloworld");
    ASSERT_EQ(NULL, tmp_err);

}



TEST(IOVecMultiPartParser, BoundaryPart){
    std::string boundary, part;
    DavixError* tmp_err = NULL;
    boundary = "gc0p4Jq0M2Yt08jU534c0p";
    part =  "--gc0p4Jq0M2Yt08jU534c0p";


    bool ret =  is_a_start_boundary_part((char*) part.c_str(), part.size(), boundary,
                                        &tmp_err);
    ASSERT_TRUE(ret );
    ASSERT_EQ(NULL, tmp_err);


    boundary = "gc0p4Jq0M2Yt08jU534c0p";
    part =  "--++gc0p4Jq0M2Yt08jU534c0p";
    ret =  is_a_start_boundary_part((char*) part.c_str(), part.size(), boundary,
                                        &tmp_err);
    ASSERT_FALSE(ret);
    ASSERT_TRUE(tmp_err != NULL);
    DavixError::clearError(&tmp_err);


}


int numb_it=0;

static int callback_offset_stupid(dav_off_t & begin, dav_off_t & end){
    begin = numb_it*1000+100;
    end=numb_it*1000+500;
    numb_it++;
    if(numb_it > 1000)
        return -1;
    return 0;
}

TEST(headerParser, generateRange){
    std::string header;
    dav_size_t size;
    dav_off_t offset;
    const dav_size_t max_header_size = rand()%8000+30;
    OffsetCallback o(callback_offset_stupid);

    std::vector<std::pair<dav_size_t,std::string> >  ranges;
    ranges = generateRangeHeaders(max_header_size, o);
    std::cout << " ranges size " << ranges.size() << std::endl;
    for(std::vector<std::pair<dav_size_t,std::string> > ::iterator it = ranges.begin(); it < ranges.end(); it++){
        ASSERT_LE(it->second.size(), max_header_size+20);
        ASSERT_GE(it->first, 1);

        std::cout << "NRange: " << (*it).first << std::endl;
        std::cout << "Range: " << (*it).second << std::endl;
    }
    ASSERT_LE(2, ranges.size());
}

