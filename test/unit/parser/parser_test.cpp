#include "parser_test.hpp"


#include <davix.hpp>
#include <fileops/httpiovec.hpp>
#include <gtest/gtest.h>

using namespace std;
using namespace Davix;

TEST(IOVecMultiPartParser, headerParser){
    std::string header;
    dav_size_t size;
    dav_off_t offset;
    header = "Content-type: application/xml"; // random generic header

    int ret = find_header_params((char*) header.c_str(), &size, &offset);
    ASSERT_EQ(ret,0);

    header = "big brother is wathing you";

    ret = find_header_params((char*) header.c_str(), &size, &offset);
    ASSERT_EQ(ret,-1);


    header = "Content-Range: bytes 600-900/8000";
    ret = find_header_params((char*) header.c_str(), &size, &offset);
    ASSERT_EQ(1,ret);
    ASSERT_EQ(301, size);
    ASSERT_EQ(600, offset);

    header = "conTent-range: bytes 600-900/8000"; // break case
    ret = find_header_params((char*) header.c_str(), &size, &offset);
    ASSERT_EQ(1,ret);
    ASSERT_EQ(301, size);
    ASSERT_EQ(600, offset);



    header = "conTent-range: bytes 600ssss-9e00/8000"; // break case
    ret = find_header_params((char*) header.c_str(), &size, &offset);
    ASSERT_EQ(ret,-1);

    header = "Content-Range: GalaticCreditStandard 600-900/8000";
    ret = find_header_params((char*) header.c_str(), &size, &offset);
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
