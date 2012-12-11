
#include <ctime>
#include <datetime/datetime_utils.h>
#include <davix.h>
#include <xml/davxmlparser.hpp>
#include <xml/davpropxmlparser.hpp>
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

const char* simple_bad_content_http =
        "lkfsdlkfdsklmkmlkmlsfdoiretopôiptrefdlzeamllvmg"
        "sfdfsjgsdmbkmlbkl,,klmd848486468+4666666666666666"
        "sfdfsjgsdmbkmlbkl,,klmd848486468+4666666666666666"
        "sfdfsjgsdmbkmlbkl,,klmd848486468+4666666666666666"
        "sfdfsjgsdmbkmlbkl,,klmd848486468+4666666666666666"
        "sfdfsjgsdmbkmlbkl,,klmd848486468+4666666666666666";


const char* recursive_listing =
        "       <D:multistatus xmlns:D=\"DAV:\">"
        "        <D:response>"
        "        <D:href>/pnfs/desy.de/data/dteam/</D:href>"
        "        <D:propstat>"
        "        <D:prop>"
        "        <D:creationdate>2012-10-22T10:50:46Z</D:creationdate>"
        "        <D:getlastmodified>Mon, 22 Oct 2012 10:50:46 GMT</D:getlastmodified>"
        "        <D:displayname>dteam</D:displayname>"
        "        <D:resourcetype>"
        "        <D:collection/></D:resourcetype>"
        "        <D:getcontenttype>text/html</D:getcontenttype>"
        "        <D:getcontentlength/><D:getetag>000033743EFFCFC64360A82A6B0A814C2C87_-2011651620</D:getetag>"
        "        </D:prop>"
        "        <D:status>HTTP/1.1 200</D:status>"
        "        </D:propstat>"
        "        </D:response>"
        "        <D:response>"
        "        <D:href>/pnfs/desy.de/data/dteam/g2/</D:href>"
        "        <D:propstat>"
        "        <D:prop>"
        "        <D:creationdate>2012-10-22T01:14:23Z</D:creationdate>"
        "        <D:getlastmodified>Mon, 22 Oct 2012 01:14:23 GMT</D:getlastmodified>"
        "        <D:displayname>g2</D:displayname>"
        "        <D:resourcetype>"
        "        <D:collection/></D:resourcetype>"
        "        <D:getcontenttype>text/html</D:getcontenttype>"
        "        <D:getcontentlength/><D:getetag>000071F56A6971D444C6B923205B8A6C8B5B_-2046234581</D:getetag>"
        "        </D:prop>"
        "        <D:status>HTTP/1.1 200</D:status>"
        "        </D:propstat>"
        "        </D:response>"
        "        <D:response>"
        "        <D:href>/pnfs/desy.de/data/dteam/generated/</D:href>"
        "        <D:propstat>"
        "        <D:prop>"
        "        <D:creationdate>2011-12-19T09:48:58Z</D:creationdate>"
        "        <D:getlastmodified>Mon, 19 Dec 2011 09:48:58 GMT</D:getlastmodified>"
        "        <D:displayname>generated</D:displayname>"
        "        <D:resourcetype>"
        "        <D:collection/></D:resourcetype>"
        "        <D:getcontenttype>text/html</D:getcontenttype>"
        "        <D:getcontentlength/><D:getetag>0000CF0EFBD2343E45099A3664C9E52A5035_1438212009</D:getetag>"
        "        </D:prop>"
        "        <D:status>HTTP/1.1 200</D:status>"
        "        </D:propstat>"
        "        </D:response>"
        "        <D:response>"
        "        <D:href>/pnfs/desy.de/data/dteam/test/</D:href>"
        "        <D:propstat>"
        "        <D:prop>"
        "        <D:creationdate>2012-03-23T14:14:35Z</D:creationdate>"
        "        <D:getlastmodified>Fri, 23 Mar 2012 14:14:35 GMT</D:getlastmodified>"
        "        <D:displayname>test</D:displayname>"
        "        <D:resourcetype>"
        "        <D:collection/></D:resourcetype>"
        "        <D:getcontenttype>text/html</D:getcontenttype>"
        "        <D:getcontentlength/><D:getetag>00002EBC0878E36B43FE98754FC725CFFA59_1072213609</D:getetag>"
        "        </D:prop>"
        "        <D:status>HTTP/1.1 200</D:status>"
        "        </D:propstat>"
        "        </D:response>"
        "        <D:response>"
        "        <D:href>/pnfs/desy.de/data/dteam/speed_test2/</D:href>"
        "        <D:propstat>"
        "        <D:prop>"
        "        <D:creationdate>2012-03-23T15:58:26Z</D:creationdate>"
        "        <D:getlastmodified>Fri, 23 Mar 2012 15:58:26 GMT</D:getlastmodified>"
        "        <D:displayname>speed_test2</D:displayname>"
        "        <D:resourcetype>"
        "        <D:collection/></D:resourcetype>"
        "        <D:getcontenttype>text/html</D:getcontenttype>"
        "        <D:getcontentlength/><D:getetag>000001C9BF5FAA6740F2BCFD7F9454C7CEF0_1078444680</D:getetag>"
        "        </D:prop>"
        "        <D:status>HTTP/1.1 200</D:status>"
        "        </D:propstat>"
        "        </D:response>"
        "        <D:response>"
        "        <D:href>/pnfs/desy.de/data/dteam/speed_test/</D:href>"
        "        <D:propstat>"
        "        <D:prop>"
        "        <D:creationdate>2012-03-23T15:44:47Z</D:creationdate>"
        "        <D:getlastmodified>Fri, 23 Mar 2012 15:44:47 GMT</D:getlastmodified>"
        "        <D:displayname>speed_test</D:displayname>"
        "        <D:resourcetype>"
        "        <D:collection/></D:resourcetype>"
        "        <D:getcontenttype>text/html</D:getcontenttype>"
        "        <D:getcontentlength/><D:getetag>000003C8148BB6574B03820B3953A1E09E3E_1077625629</D:getetag>"
        "        </D:prop>"
        "        <D:status>HTTP/1.1 200</D:status>"
        "        </D:propstat>"
        "        </D:response>"
        "        <D:response>"
        "        <D:href>/pnfs/desy.de/data/dteam/speed_test3/</D:href>"
        "        <D:propstat>"
        "        <D:prop>"
        "        <D:creationdate>2012-03-23T16:55:34Z</D:creationdate>"
        "        <D:getlastmodified>Fri, 23 Mar 2012 16:55:34 GMT</D:getlastmodified>"
        "        <D:displayname>speed_test3</D:displayname>"
        "        <D:resourcetype>"
        "        <D:collection/></D:resourcetype>"
        "        <D:getcontenttype>text/html</D:getcontenttype>"
        "        <D:getcontentlength/><D:getetag>0000C0F214B189464616B08D01F540445829_1081872745</D:getetag>"
        "        </D:prop>"
        "        <D:status>HTTP/1.1 200</D:status>"
        "        </D:propstat>"
        "        </D:response>"
        "        <D:response>"
        "        <D:href>/pnfs/desy.de/data/dteam/testwrite</D:href>"
        "        <D:propstat>"
        "        <D:prop>"
        "        <D:creationdate>2012-03-23T16:47:11Z</D:creationdate>"
        "        <D:getlastmodified>Fri, 23 Mar 2012 16:47:11 GMT</D:getlastmodified>"
        "        <D:displayname>testwrite</D:displayname>"
        "        <D:resourcetype/><D:getcontenttype/><D:getcontentlength>65536</D:getcontentlength>"
        "        <D:getetag>0000DA13494AE9764029BE48D6D264AF2E0C_1081369798</D:getetag>"
        "        </D:prop>"
        "        <D:status>HTTP/1.1 200</D:status>"
        "        </D:propstat>"
        "        </D:response>"
        "        <D:response>"
        "        <D:href>/pnfs/desy.de/data/dteam/test_dir/</D:href>"
        "        <D:propstat>"
        "        <D:prop>"
        "        <D:creationdate>2012-05-24T16:22:28Z</D:creationdate>"
        "        <D:getlastmodified>Thu, 24 May 2012 16:22:28 GMT</D:getlastmodified>"
        "        <D:displayname>test_dir</D:displayname>"
        "        <D:resourcetype>"
        "        <D:collection/></D:resourcetype>"
        "        <D:getcontenttype>text/html</D:getcontenttype>"
        "        <D:getcontentlength/><D:getetag>000066C9BA492C3440E081A6CF0A75479B98_2141719820</D:getetag>"
        "        </D:prop>"
        "        <D:status>HTTP/1.1 200</D:status>"
        "        </D:propstat>"
        "        </D:response>"
        "        <D:response>"
        "        <D:href>/pnfs/desy.de/data/dteam/testgfgfgfdg9999tw3</D:href>"
        "        <D:propstat>"
        "        <D:prop>"
        "        <D:creationdate>2012-07-13T13:25:01Z</D:creationdate>"
        "        <D:getlastmodified>Fri, 13 Jul 2012 13:25:02 GMT</D:getlastmodified>"
        "        <D:displayname>testgfgfgfdg9999tw3</D:displayname>"
        "        <D:resourcetype/><D:getcontenttype/><D:getcontentlength>187</D:getcontentlength>"
        "        <D:getetag>0000AF211A13EC96400393F4769AC24CD82F_-2138860744</D:getetag>"
        "        </D:prop>"
        "        <D:status>HTTP/1.1 200</D:status>"
        "        </D:propstat>"
        "        </D:response>"
        "        <D:response>"
        "        <D:href>/pnfs/desy.de/data/dteam/UGRtest/</D:href>"
        "        <D:propstat>"
        "        <D:prop>"
        "        <D:creationdate>2012-07-19T14:50:51Z</D:creationdate>"
        "        <D:getlastmodified>Thu, 19 Jul 2012 14:50:51 GMT</D:getlastmodified>"
        "        <D:displayname>UGRtest</D:displayname>"
        "        <D:resourcetype>"
        "        <D:collection/></D:resourcetype>"
        "        <D:getcontenttype>text/html</D:getcontenttype>"
        "        <D:getcontentlength/><D:getetag>00009BF2D07746D14CB4BEB74097309AB707_-1615312788</D:getetag>"
        "        </D:prop>"
        "        <D:status>HTTP/1.1 200</D:status>"
        "        </D:propstat>"
        "        </D:response>"
        "        <D:response>"
        "        <D:href>/pnfs/desy.de/data/dteam/ugrtest/</D:href>"
        "        <D:propstat>"
        "        <D:prop>"
        "        <D:creationdate>2012-07-19T14:53:42Z</D:creationdate>"
        "        <D:getlastmodified>Wed, 25 Jul 2012 10:01:02 GMT</D:getlastmodified>"
        "        <D:displayname>ugrtest</D:displayname>"
        "        <D:resourcetype>"
        "        <D:collection/></D:resourcetype>"
        "        <D:getcontenttype>text/html</D:getcontenttype>"
        "        <D:getcontentlength/><D:getetag>0000D40132559B244F7597EDB2EAE617D56B_-1114301154</D:getetag>"
        "        </D:prop>"
        "        <D:status>HTTP/1.1 200</D:status>"
        "        </D:propstat>"
        "        </D:response>"
        "        <D:response>"
        "        <D:href>/pnfs/desy.de/data/dteam/fbxtest.txt</D:href>"
        "        <D:propstat>"
        "        <D:prop>"
        "        <D:creationdate>2012-07-19T14:38:31Z</D:creationdate>"
        "        <D:getlastmodified>Thu, 19 Jul 2012 14:38:33 GMT</D:getlastmodified>"
        "        <D:displayname>fbxtest.txt</D:displayname>"
        "        <D:resourcetype/><D:getcontenttype>text/plain</D:getcontenttype>"
        "        <D:getcontentlength>640999</D:getcontentlength>"
        "        <D:getetag>00004769A9ACEB1647EF8C8D6298FE1D5909_-1616050534</D:getetag>"
        "        </D:prop>"
        "        <D:status>HTTP/1.1 200</D:status>"
        "        </D:propstat>"
        "        </D:response>"
        "        <D:response>"
        "        <D:href>/pnfs/desy.de/data/dteam/testdir/</D:href>"
        "        <D:propstat>"
        "        <D:prop>"
        "        <D:creationdate>2012-07-19T14:39:05Z</D:creationdate>"
        "        <D:getlastmodified>Thu, 19 Jul 2012 14:39:05 GMT</D:getlastmodified>"
        "        <D:displayname>testdir</D:displayname>"
        "        <D:resourcetype>"
        "        <D:collection/></D:resourcetype>"
        "        <D:getcontenttype>text/html</D:getcontenttype>"
        "        <D:getcontentlength/><D:getetag>0000CA3E0FFC5E684F639541654D7FD99B72_-1616018106</D:getetag>"
        "        </D:prop>"
        "        <D:status>HTTP/1.1 200</D:status>"
        "        </D:propstat>"
        "        </D:response>"
        "        <D:response>"
        "        <D:href>/pnfs/desy.de/data/dteam/testdir8888/</D:href>"
        "        <D:propstat>"
        "        <D:prop>"
        "        <D:creationdate>2012-07-19T14:39:16Z</D:creationdate>"
        "        <D:getlastmodified>Thu, 19 Jul 2012 14:39:16 GMT</D:getlastmodified>"
        "        <D:displayname>testdir8888</D:displayname>"
        "        <D:resourcetype>"
        "        <D:collection/></D:resourcetype>"
        "        <D:getcontenttype>text/html</D:getcontenttype>"
        "        <D:getcontentlength/><D:getetag>0000BFDBF3DE079E407F92396F58D65FE656_-1616006747</D:getetag>"
        "        </D:prop>"
        "           <D:status>HTTP/1.1 200</D:status>"
        "            </D:propstat>"
        "            </D:response>"
        "            <D:response>"
        "            <D:href>/pnfs/desy.de/data/dteam/testdir8889/</D:href>"
        "            <D:propstat>"
        "            <D:prop>"
        "            <D:creationdate>2012-07-19T14:51:35Z</D:creationdate>"
        "            <D:getlastmodified>Thu, 19 Jul 2012 14:51:35 GMT</D:getlastmodified>"
        "            <D:displayname>testdir8889</D:displayname>"
        "            <D:resourcetype>"
        "            <D:collection/></D:resourcetype>"
        "            <D:getcontenttype>text/html</D:getcontenttype>"
        "            <D:getcontentlength/><D:getetag>0000DB00326864EC43529E1CE8137A4F9A00_-1615267437</D:getetag>"
        "            </D:prop>"
        "            <D:status>HTTP/1.1 200</D:status>"
        "            </D:propstat>"
        "            </D:response>"
        "            </D:multistatus>";


const char* list_item[] = { "g2", "generated", "test", "speed_test2", "speed_test", "speed_test3", "testwrite", "test_dir",
                            "testgfgfgfdg9999tw3", "UGRtest", "ugrtest", "fbxtest.txt", "testdir", "testdir8888", "testdir8889" };

TEST(XmlParserInstance, createParser){
    Davix::DavXMLParser * parser = new Davix::DavXMLParser();
    Davix::DavixError* last_error = parser->getLastErr();
    ASSERT_EQ(DAVIX_STATUS_OK,last_error->getStatus());
    delete parser;
    Davix::DavixError::clearError(&last_error);
}


TEST(XmlParserInstance, parseOneStat){
    davix_set_log_level(DAVIX_LOG_ALL);
    Davix::DavPropXMLParser parser;

    int ret = parser.parseChuck(simple_stat_propfind_content, strlen(simple_stat_propfind_content));
    if( ret !=0){
        std::cerr << " error : " << parser.getLastErr()->getErrMsg() << std::endl;
        ASSERT_TRUE(FALSE);
    }
    ASSERT_EQ(1u, parser.getProperties().size());
    parser.parseChuck(NULL, 0);
    ASSERT_EQ(1u, parser.getProperties().size());

    Davix::FileProperties f = parser.getProperties().at(0);
    ASSERT_TRUE(S_ISDIR(f.mode));
    ASSERT_FALSE(S_ISLNK(f.mode));
    ASSERT_STREQ("dteam",f.filename.c_str());
 //q   ASSERT_EQ(f.mtime, 1350892251L);

}


TEST(XMLParserInstance,ParseList){
    Davix::DavPropXMLParser parser;

    int ret = parser.parseChuck(recursive_listing, strlen(recursive_listing));
    if( ret !=0){
        std::cerr << " error : " << parser.getLastErr()->getErrMsg() << std::endl;
        ASSERT_TRUE(FALSE);
    }
    ASSERT_EQ(16u, parser.getProperties().size());
    parser.parseChuck(NULL, 0);
    ASSERT_EQ(16u, parser.getProperties().size());

    // test the parent directory stats
    Davix::FileProperties f = parser.getProperties().at(0);
    ASSERT_TRUE(S_ISDIR(f.mode));
    ASSERT_FALSE(S_ISLNK(f.mode));
    ASSERT_STREQ("dteam",f.filename.c_str());

    for(int i =1; i < 16; ++i){
        Davix::FileProperties f_local = parser.getProperties()[i];
        ASSERT_STREQ(list_item[i-1], f_local.filename.c_str());
        ASSERT_TRUE( f_local.size > 0 || S_ISDIR(f.mode));
    }

    // test the children stats


}

TEST(XmlParserInstance,parserNonWebdav){
    Davix::DavPropXMLParser parser;

    int ret = parser.parseChuck(simple_bad_content_http, strlen(simple_bad_content_http));
    parser.parseChuck(NULL, 0);

    ASSERT_EQ(-1, ret);
    Davix::DavixError * last_error = parser.getLastErr();
    std::cerr << "error : " << last_error->getErrMsg();
    ASSERT_EQ(0u, parser.getProperties().size());
    ASSERT_TRUE(NULL != last_error);
    ASSERT_EQ(Davix::StatusCode::WebDavPropertiesParsingError, last_error->getStatus());
    Davix::DavixError::clearError(&last_error);
}


TEST(XmlPaserInstance, destroyPartial){
    davix_set_log_level(DAVIX_LOG_ALL);
    Davix::DavPropXMLParser* parser = new Davix::DavPropXMLParser();

    int ret = parser->parseChuck(simple_stat_propfind_content, strlen(simple_stat_propfind_content)/2);
    if( ret !=0){
        std::cerr << " error : " << parser->getLastErr()->getErrMsg() << std::endl;
        ASSERT_TRUE(FALSE);
    }
    // destroy the parser with still parsing on the stack
    delete parser;
}






