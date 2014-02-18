#include "parser_test.hpp"


#include <davix.hpp>
#include <fileops/httpiovec.hpp>
#include <fileops/fileutils.hpp>
#include <gtest/gtest.h>
#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/filestream.h>

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


// JSON parser tutorial test
TEST(ParserJSONTestTuto, JSONTuto){



        ////////////////////////////////////////////////////////////////////////////
        // 1. Parse a JSON text string to a document.

        const char json[] = " { \"hello\" : \"world\", \"t\" : true , \"f\" : false, \"n\": null, \"i\":123, \"pi\": 3.1416, \"a\":[1, 2, 3, 4] } ";
        //printf("Original JSON:\n %s\n", json);

        rapidjson::Document document;	// Default template parameter uses UTF8 and MemoryPoolAllocator.

    #if 0
        // "normal" parsing, decode strings to new buffers. Can use other input stream via ParseStream().
        if (document.Parse<0>(json).HasParseError())
            ASSERT_TRUE(false);
    #else
        // In-situ parsing, decode strings directly in the source string. Source must be string.
        char buffer[sizeof(json)];
        memcpy(buffer, json, sizeof(json));
        if (document.ParseInsitu<0>(buffer).HasParseError())
            ASSERT_TRUE(false);
    #endif

       // printf("\nParsing to document succeeded.\n");


}


// JSON parser tutorial test
TEST(ParserJSONTestTuto, JSONReplicaParsing){



        ////////////////////////////////////////////////////////////////////////////
        // 1. Parse a JSON text string to a document.

        const char json[] = "["
                "{"
                "\"server\"    : \"datagrid.lbl.gov\","
                "\"rfn\"       : \"http://datagrid.lbl.gov/testdata//R/test01.data\","
                "\"atime\"     : 1384793007,"
                "\"status\"    : \"-\","
                "\"type\"      : \"V\","
                "\"ltime\"     : 1385397803,"
                "\"extra\": {}"
                "}"
                "]";
        //printf("Original JSON:\n %s\n", json);

        rapidjson::Document document;	// Default template parameter uses UTF8 and MemoryPoolAllocator.

    #if 0
        // "normal" parsing, decode strings to new buffers. Can use other input stream via ParseStream().
        if (document.Parse<0>(json).HasParseError())
            ASSERT_TRUE(false);
    #else
        // In-situ parsing, decode strings directly in the source string. Source must be string.
        char buffer[sizeof(json)];
        memcpy(buffer, json, sizeof(json));
        if (document.ParseInsitu<0>(buffer).HasParseError())
            ASSERT_TRUE(false);
    #endif

        //printf("\nParsing to replicas with success. \n");

        ASSERT_TRUE(document.IsArray());
        ASSERT_EQ(document.Size(),1);
        ASSERT_TRUE(document[static_cast<rapidjson::SizeType>(0)].IsObject());
        ASSERT_FALSE(document[static_cast<rapidjson::SizeType>(0)].HasMember("test_random"));
        ASSERT_TRUE(document[static_cast<rapidjson::SizeType>(0)].HasMember("rfn"));
        ASSERT_STREQ("http://datagrid.lbl.gov/testdata//R/test01.data", document[static_cast<rapidjson::SizeType>(0)]["rfn"].GetString());
        ASSERT_STREQ("datagrid.lbl.gov", document[static_cast<rapidjson::SizeType>(0)]["server"].GetString());
}




// JSON parser tutorial test
TEST(UriTests, testRelativeUri){
    Davix::Uri u("http://datagrid.lbl.gov/testdata/R/test01.data");
    std::string proto_rel("//example.org/test"), abs_path("/hello/world/"), rel_path("blabla/test");

    Davix::Uri res = Uri::fromRelativePath(u, proto_rel);
    ASSERT_EQ(StatusCode::OK, res.getStatus());
    ASSERT_STREQ("http://example.org/test", res.getString().c_str());

    res = Uri::fromRelativePath(u, abs_path);
    ASSERT_EQ(StatusCode::OK, res.getStatus());
    ASSERT_STREQ("http://datagrid.lbl.gov/hello/world/", res.getString().c_str());

    res = Uri::fromRelativePath(u, rel_path);
    ASSERT_EQ(StatusCode::OK, res.getStatus());
    ASSERT_STREQ("http://datagrid.lbl.gov/testdata/R/test01.data/blabla/test", res.getString().c_str());

}





