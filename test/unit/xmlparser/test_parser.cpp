
#include <ctime>
#include <datetime/datetime_utils.h>
#include <davix.h>
#include <xmlpp/davxmlparser.hpp>
#include <xmlpp/davpropxmlparser.hpp>
#include <gtest/gtest.h>
#include <string.h>


const char* simple_stat_propfind_content =
        "<?xml version=\"1.0\" encoding=\"utf-8\" ?>"
        "<D:multistatus xmlns:D=\"DAV:\">"
        "<D:response>"
        "<D:href>/pnfs/desy.de/data/dteam/</D:href>"
        "<D:propstat>"
        "<D:prop>"
        "<D:creationdate>2012-10-22T07:50:51Z</D:creationdate>"
        "<D:getlastmodified>Mon, 22 Oct 2012 07:50:51 GMT</D:getlastmodified>"
        "<D:displayname>dteam</D:displayname>"
        "<D:resourcetype>"
        "<D:collection/></D:resourcetype>"
        "<D:getcontenttype>text/html</D:getcontenttype>"
        "<D:getcontentlength/><D:getetag>000033743EFFCFC64360A82A6B0A814C2C87_-2022446850</D:getetag>"
        "</D:prop>"
        "<D:status>HTTP/1.1 200</D:status>"
        "</D:propstat>"
        "</D:response>"
        "</D:multistatus>";


TEST(XmlParserInstance, createParser){
    Davix::DavXMLParser * parser = new Davix::DavXMLParser();
    ASSERT_EQ(NULL,parser->getLastErr());
    delete parser;
}


TEST(XmlPaserInstance, parseOneStat){
    g_logger_set_globalfilter(G_LOG_LEVEL_MASK);
    Davix::DavPropXMLParser parser;

    int ret = parser.parseChuck(simple_stat_propfind_content, strlen(simple_stat_propfind_content));
    if( ret !=0){
        std::cerr << " error : " << parser.getLastErr()->getErrMsg() << std::endl;
        ASSERT_TRUE(FALSE);
    }
    ASSERT_EQ(1, parser.getProperties().size());
    parser.parseChuck(NULL, 0);
    ASSERT_EQ(1, parser.getProperties().size());

    Davix::FileProperties f = parser.getProperties().at(0);
    ASSERT_TRUE(S_ISDIR(f.mode));
    ASSERT_FALSE(S_ISLNK(f.mode));
    ASSERT_STREQ("dteam",f.filename.c_str());
    ASSERT_EQ(f.mtime, 1350892251L);

}






