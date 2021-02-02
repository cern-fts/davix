#include <typeinfo>
#include <gtest/gtest.h>
#include <davix_internal_config.hpp>
#include <stdbool.h>
#include <ctime>
#include "libs/datetime/datetime_utils.hpp"
#include <davix.hpp>
#include <xml/davxmlparser.hpp>
#include <xml/davpropxmlparser.hpp>
#include <xml/metalinkparser.hpp>
#include <xml/s3propparser.hpp>
#include <xml/S3MultiPartInitiationParser.hpp>
#include <xml/swiftpropparser.hpp>
#include <status/davixstatusrequest.hpp>
#include <string.h>
#include <xml/davix_ptree.hpp>


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

const char * caldav_item = "<?xml version=\"1.0\" encoding=\"utf-8\" ?>"
        "<d:multistatus xmlns:cs=\"http://calendarserver.org/ns/\" xmlns:d=\"DAV:\" xmlns:cal=\"urn:ietf:params:xml:ns:caldav\" xmlns:card=\"urn:ietf:params:xml:ns:carddav\"><d:response><d:href>/dteam/</d:href><d:propstat><d:prop><d:getcontentlength/><d:getlastmodified>Wed, 18 Dec 2013 14:41:52 GMT</d:getlastmodified><d:resourcetype><d:collection/></d:resourcetype><d:displayname>dteam</d:displayname><d:creationdate>2013-12-18T14:41:52Z</d:creationdate></d:prop><d:status>HTTP/1.1 200 OK</d:status></d:propstat><d:propstat><d:prop><mode/></d:prop><d:status>HTTP/1.1 404 Not Found</d:status></d:propstat></d:response><d:response><d:href>/dteam/foo2.txt</d:href><d:propstat><d:prop><d:getcontentlength>12</d:getcontentlength><d:getlastmodified>Sat, 16 Jul 2011 17:03:02 GMT</d:getlastmodified><d:resourcetype/><d:displayname>foo2.txt</d:displayname><d:creationdate>2011-07-16T17:03:02Z</d:creationdate></d:prop><d:status>HTTP/1.1 200 OK</d:status></d:propstat><d:propstat><d:prop><mode/></d:prop><d:status>HTTP/1.1 404 Not Found</d:status></d:propstat></d:response><d:response><d:href>/dteam/foo.txt</d:href><d:propstat><d:prop><d:getcontentlength>12</d:getcontentlength><d:getlastmodified>Sat, 16 Jul 2011 06:30:51 GMT</d:getlastmodified><d:resourcetype/><d:displayname>foo.txt</d:displayname><d:creationdate>2011-07-16T06:30:51Z</d:creationdate></d:prop><d:status>HTTP/1.1 200 OK</d:status></d:propstat><d:propstat><d:prop><mode/></d:prop><d:status>HTTP/1.1 404 Not Found</d:status></d:propstat></d:response><d:response><d:href>/dteam/EA-check-file-N_1-1311262879444</d:href><d:propstat><d:prop><d:getcontentlength>0</d:getcontentlength><d:getlastmodified>Thu, 21 Jul 2011 15:41:19 GMT</d:getlastmodified><d:resourcetype/><d:displayname>EA-check-file-N_1-1311262879444</d:displayname><d:creationdate>2011-07-21T15:41:19Z</d:creationdate></d:prop><d:status>HTTP/1.1 200 OK</d:status></d:propstat><d:propstat><d:prop><mode/></d:prop><d:status>HTTP/1.1 404 Not Found</d:status></d:propstat></d:response><d:response><d:href>/dteam/fpalapzorp</d:href><d:propstat><d:prop><d:getcontentlength>1286144</d:getcontentlength><d:getlastmodified>Wed, 14 Sep 2011 11:43:41 GMT</d:getlastmodified><d:resourcetype/><d:displayname>fpalapzorp</d:displayname><d:creationdate>2011-09-14T11:43:41Z</d:creationdate></d:prop><d:status>HTTP/1.1 200 OK</d:status></d:propstat><d:propstat><d:prop><mode/></d:prop><d:status>HTTP/1.1 404 Not Found</d:status></d:propstat></d:response><d:response><d:href>/dteam/EA-check-file-N_1-1314125987590</d:href><d:propstat><d:prop><d:getcontentlength>0</d:getcontentlength><d:getlastmodified>Tue, 23 Aug 2011 18:59:47 GMT</d:getlastmodified><d:resourcetype/><d:displayname>EA-check-file-N_1-1314125987590</d:displayname><d:creationdate>2011-08-23T18:59:47Z</d:creationdate></d:prop><d:status>HTTP/1.1 200 OK</d:status></d:propstat><d:propstat><d:prop><mode/></d:prop><d:status>HTTP/1.1 404 Not Found</d:status></d:propstat></d:response><d:response><d:href>/dteam/somefilename</d:href><d:propstat><d:prop><d:getcontentlength>1286144</d:getcontentlength><d:getlastmodified>Wed, 14 Sep 2011 14:16:03 GMT</d:getlastmodified><d:resourcetype/><d:displayname>somefilename</d:displayname><d:creationdate>2011-09-14T14:16:03Z</d:creationdate></d:prop><d:status>HTTP/1.1 200 OK</d:status></d:propstat><d:propstat><d:prop><mode/></d:prop><d:status>HTTP/1.1 404 Not Found</d:status></d:propstat></d:response><d:response><d:href>/dteam/generated</d:href><d:propstat><d:prop><d:getcontentlength/><d:getlastmodified>Sat, 26 Feb 2011 14:39:48 GMT</d:getlastmodified><d:resourcetype><d:collection/></d:resourcetype><d:displayname>generated</d:displayname><d:creationdate>2011-02-26T14:39:48Z</d:creationdate></d:prop><d:status>HTTP/1.1 200 OK</d:status></d:propstat><d:propstat><d:prop><mode/></d:prop><d:status>HTTP/1.1 404 Not Found</d:status></d:propstat></d:response></d:multistatus>";



const char* list_item[] = { "g2", "generated", "test", "speed_test2", "speed_test", "speed_test3", "testwrite", "test_dir",
                            "testgfgfgfdg9999tw3", "UGRtest", "ugrtest", "fbxtest.txt", "testdir", "testdir8888", "testdir8889" };


const char metalink_item_lcgdm[] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<metalink version=\"3.0\" xmlns=\"http://www.metalinker.org/\" xmlns:lcgdm=\"LCGDM:\" generator=\"lcgdm-dav\" pubdate=\"Tue, 13 Feb 2007 20:17:47 GMT\">"
        "<files>"
        "<file name=\"/emidem\">"
        "	<size>494391600</size>"
        "	<resources>"
        "		<url type=\"https\">http://datagrid.lbl.gov/testdata//L/test02.data</url>"
        "	</resources>"
        "</file>"
        "</files>"
        "</metalink>";


const char metalink_item_generic[]= " "
        "<metalink version=\"3.0\" xmlns=\"http://www.metalinker.org/\">"
        "    <files>"
        "      <file name=\"example.ext\">"
        "      <verification>"
        "        <hash type=\"md5\">example-md5-hash</hash>"
        "        <hash type=\"sha1\">example-sha1-hash</hash>"
        "      </verification>"
        "      <resources>"
        "        <url type=\"ftp\">ftp://ftp.example1.com/example.ext</url>"
        "        <url type=\"ftp\">ftp://ftp.example2.com/example.ext</url>"
        "        <url type=\"http\">http://www.example1.com/example.ext</url> "
        "        <url type=\"http\">http://www.example2.com/example.ext</url>"
        "        <url type=\"http\">http://www.example3.com/example.ext</url> "
        "        <url type=\"bittorrent\">http://www.ex.com/example.ext.torrent</url>"
        "        <url type=\"magnet\"/>"
        "        <url type=\"ed2k\"/>"
        "      </resources>"
        "      </file>"
        "    </files>"
        "  </metalink>";


const std::string s3_xml_response = "<?xml version=\"1.0\" encoding=\"UTF-8\"?><ListBucketResult xmlns=\"http://s3.amazonaws.com/doc/2006-03-01/\"><Name>a-random-random-bucket</Name><Prefix></Prefix><Marker></Marker><MaxKeys>1000</MaxKeys><IsTruncated>false</IsTruncated><Contents><Key>h1big.root</Key><LastModified>2014-09-19T14:27:33.000Z</LastModified><ETag>&quot;bf5b1efa7fe677965bf3ecd41e20be2a&quot;</ETag><Size>280408881</Size><StorageClass>STANDARD</StorageClass><Owner><ID>mhellmic</ID><DisplayName>Martin Hellmich</DisplayName></Owner></Contents><Contents><Key>services</Key><LastModified>2014-10-03T14:58:12.000Z</LastModified><ETag>&quot;3e73cc5c77799fd3e7a02c62474107bb&quot;</ETag><Size>\t   19558   \t</Size><StorageClass>STANDARD</StorageClass><Owner><ID>mhellmic</ID><DisplayName>Martin Hellmich</DisplayName></Owner></Contents></ListBucketResult>";

const std::string s3_multipart_initiation_response = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
"<InitiateMultipartUploadResult"
" xmlns=\"http://s3.amazonaws.com/doc/2006-03-01/\">"
"   <Bucket>example-bucket</Bucket>"
"   <Key>example-object</Key>"
"   <UploadId>EXAMPLEJZ6e0YupT2h66iePQCc9IEbYbDUy4RTpMeoSMLPRp8Z5o1u8feSRonpvnWsKKG35tI2LB9VDPiCgTy.Gq2VxQLYjrue4Nq.NBdqI-</UploadId>"
"</InitiateMultipartUploadResult>  ";

const std::string swift_xml_response = "<?xml version=\"1.0\" encoding=\"UTF-8\"?><container name=\"backups\"><subdir name=\"photos/animals/\"><name>photos/animals/</name></subdir><object><name>photos/me.jpg</name><hash>b249a153f8f38b51e92916bbc6ea57ad</hash><bytes>2906</bytes><content_type>image/jpeg</content_type><last_modified>2015-12-03T17:31:28.187370</last_modified></object><subdir name=\"photos/plants/\"><name>photos/plants/</name></subdir></container>";

TEST(XmlParserInstance, createParser){

    ASSERT_NO_THROW({
    Davix::XMLSAXParser * parser = new Davix::XMLSAXParser();
    delete parser;
    });
}


TEST(XmlParserInstance, parseOneStat){
    davix_set_log_level(DAVIX_LOG_ALL);
    Davix::DavPropXMLParser parser;

    ASSERT_NO_THROW({

    int ret = parser.parseChunk(simple_stat_propfind_content, strlen(simple_stat_propfind_content));
    if( ret !=0){
        ASSERT_TRUE(false);
    }
    ASSERT_EQ(1u, parser.getProperties().size());
    parser.parseChunk(NULL, 0);
    ASSERT_EQ(1u, parser.getProperties().size());

    Davix::FileProperties f = parser.getProperties().at(0);
    ASSERT_TRUE(S_ISDIR(f.info.mode));
    ASSERT_FALSE(S_ISLNK(f.info.mode));
    ASSERT_STREQ("dteam",f.filename.c_str());
 //q   ASSERT_EQ(f.mtime, 1350892251L);

    });

}


TEST(XMLParserInstance,ParseList){


    ASSERT_NO_THROW({
        Davix::DavPropXMLParser parser;
        int ret = parser.parseChunk(recursive_listing, strlen(recursive_listing));
        if( ret !=0){
            ASSERT_TRUE(false);
        }
        ASSERT_EQ(16u, parser.getProperties().size());
        parser.parseChunk(NULL, 0);
        ASSERT_EQ(16u, parser.getProperties().size());

        // test the parent directory stats
        Davix::FileProperties f = parser.getProperties().at(0);
        ASSERT_TRUE(S_ISDIR(f.info.mode));
        ASSERT_FALSE(S_ISLNK(f.info.mode));
        ASSERT_STREQ("dteam",f.filename.c_str());

        for(int i =1; i < 16; ++i){
            Davix::FileProperties f_local = parser.getProperties()[i];
            ASSERT_STREQ(list_item[i-1], f_local.filename.c_str());
            ASSERT_TRUE( f_local.info.size > 0 || S_ISDIR(f.info.mode));
        }

        // test the children stats

    });
}


TEST(XMLParserInstance, ParseCalDav){


    ASSERT_NO_THROW({
        davix_set_log_level(DAVIX_LOG_ALL);
        Davix::DavPropXMLParser parser;

        int ret = parser.parseChunk(caldav_item, strlen(caldav_item));
        if( ret !=0){
            ASSERT_TRUE(false);
        }
        ASSERT_GT(parser.getProperties().size(),0);

    });
}

TEST(XmlParserInstance,parserNonWebdav){
    Davix::DavPropXMLParser parser;

    try{
        parser.parseChunk(simple_bad_content_http, strlen(simple_bad_content_http));
        parser.parseChunk(NULL, 0);
        ASSERT_TRUE(false);
    }catch(Davix::DavixException & e){

        std::cerr << "error : " << e.what();
        ASSERT_EQ(0u, parser.getProperties().size());
        ASSERT_TRUE(Davix::StatusCode::OK != e.code());
        ASSERT_EQ(Davix::StatusCode::WebDavPropertiesParsingError, e.code());
    }
}


TEST(XmlPaserInstance, destroyPartial){
    davix_set_log_level(DAVIX_LOG_ALL);
    Davix::DavPropXMLParser* parser = new Davix::DavPropXMLParser();

    ASSERT_NO_THROW({

        int ret = parser->parseChunk(simple_stat_propfind_content, strlen(simple_stat_propfind_content)/2);
        if( ret !=0){
            ASSERT_TRUE(false);
        }
        // destroy the parser with still parsing on the stack
        });
    delete parser;
}


TEST(XmlMetalinkParserTest, parserMetalinkSimpl){

    ASSERT_NO_THROW({
        Davix::Context c;
        std::vector<Davix::File> r;
        Davix::MetalinkParser parser(c, r);
        int ret = parser.parseChunk(metalink_item_lcgdm, strlen(metalink_item_lcgdm));
        ASSERT_EQ(0, ret);

        ASSERT_EQ(1, r.size());
        Davix::Uri u = r[0].getUri();
        ASSERT_EQ(Davix::StatusCode::OK, u.getStatus());
        ASSERT_STREQ("http://datagrid.lbl.gov/testdata//L/test02.data", u.getString().c_str());
        ASSERT_EQ(494391600, parser.getSize());
    });
}


TEST(XmlMetalinkParserTest, parserMetalinkGeneric){

    ASSERT_NO_THROW({
    Davix::Context c;
    std::vector<Davix::File> r;
    Davix::MetalinkParser parser(c, r);
    int ret = parser.parseChunk(metalink_item_generic, strlen(metalink_item_generic));
    //std::cout << parser.getLastErr()->getErrMsg();
    ASSERT_EQ(0, ret);

   // const Davix::Properties& p = parser.getProps();
    ASSERT_EQ(8, r.size());
    Davix::Uri u = r[0].getUri();
    ASSERT_EQ(Davix::StatusCode::OK, u.getStatus());
    ASSERT_STREQ("ftp://ftp.example1.com/example.ext", u.getString().c_str());
    u = r[1].getUri();
    ASSERT_STREQ("ftp://ftp.example2.com/example.ext", u.getString().c_str());
    ASSERT_STREQ("ftp",u.getProtocol().c_str());
    });
}


TEST(XmlPTreeTest, testPTreeBase){
    using namespace Davix::Xml;
    XmlPTree base(Attribute,"start", XmlPTree::ChildrenList(1, XmlPTree(Attribute, "hello", XmlPTree::ChildrenList(1, XmlPTree(CData, "BoB")))));
    XmlPTree baseItem(Attribute, "start");
    XmlPTree randomItem(Attribute, "no");

    XmlPTree elem1(CData, "1"), elem2(CData, "2"), elemBob(CData, "BoB");
    XmlPTree::ChildrenList l;
    l.push_back(elem1); l.push_back(elem2); l.push_back(elemBob);
    XmlPTree hello(Attribute, "hello", l), nihao(Attribute, "你好", XmlPTree::ChildrenList(1, elem1));
    XmlPTree tree(Attribute, "start", XmlPTree::ChildrenList(1, hello));

    ASSERT_TRUE( base.compareKey(baseItem));
    ASSERT_TRUE( baseItem.compareKey(randomItem));
    ASSERT_TRUE(base.compareNode(baseItem));
    ASSERT_FALSE( randomItem.compareNode(baseItem));
    ASSERT_FALSE(elemBob.compareNode(elem1));


    ASSERT_TRUE( tree.matchTree(base));
    ASSERT_FALSE( base.matchTree(tree));
    ASSERT_FALSE( tree.matchTree(nihao));
}


TEST(XmlPTreeTest, testPTreeChain){
    using namespace Davix::Xml;
    XmlPTree base(Attribute,"start", XmlPTree::ChildrenList(1, XmlPTree(Attribute, "hello", XmlPTree::ChildrenList(1, XmlPTree(CData, "BoB")))));

    XmlPTree baseItem(Attribute, "start");
    XmlPTree randomItem(Attribute, "no");
    XmlPTree helloItem(Attribute, "hello");
    XmlPTree bobItem(CData, "BoB");

    std::vector<XmlPTree> vec_node;
    std::vector<XmlPTree::ptr_type> res;
    res = base.findChain(vec_node);
    ASSERT_EQ(0, res.size());
    vec_node.clear();


    vec_node.push_back(baseItem);
    res = base.findChain(vec_node);
    ASSERT_EQ(1, res.size());
    ASSERT_TRUE(res.at(0)->compareNode(baseItem));
    ASSERT_EQ(res.at(0), &(base));
    vec_node.clear();


    // invalid nodes
    vec_node.push_back(randomItem);
    res = base.findChain(vec_node);
    ASSERT_EQ(0, res.size());
    vec_node.clear();


    vec_node.push_back(baseItem);
    vec_node.push_back(helloItem);
    vec_node.push_back(bobItem);
    res = base.findChain(vec_node);
    ASSERT_EQ(3, res.size());
    ASSERT_TRUE(res.at(0)->compareNode(baseItem));
    ASSERT_TRUE(res.at(1)->compareNode(helloItem));
    ASSERT_TRUE(res.at(2)->compareNode(bobItem));
    ASSERT_EQ(res.at(0), &(base));
    ASSERT_EQ(res.at(1), &(*base.beginChildren()));
    ASSERT_EQ(res.at(2), &(*base.beginChildren()->beginChildren()));
    vec_node.clear();

    vec_node.push_back(base);
    res = baseItem.findChain(vec_node);
    ASSERT_EQ(1, res.size());
    ASSERT_TRUE(res.at(0)->compareNode(baseItem));
    vec_node.clear();


}

TEST(XmlS3parsing, TestListingBucket){
    using namespace Davix;

    davix_set_log_level(DAVIX_LOG_ALL);
    S3PropParser parser;

    int ret = parser.parseChunk(s3_xml_response);
    ASSERT_EQ(ret, 0);

    ASSERT_EQ(3, parser.getProperties().size());

    // verify name
    ASSERT_EQ(std::string("a-random-random-bucket"), parser.getProperties().at(0).filename);
    ASSERT_EQ(std::string("h1big.root"), parser.getProperties().at(1).filename);
    ASSERT_EQ(std::string("services"), parser.getProperties().at(2).filename);

    // verify size
    ASSERT_EQ(280408881, parser.getProperties().at(1).info.size);
    ASSERT_EQ(19558, parser.getProperties().at(2).info.size);
}

TEST(XmlMultiPartUploadInitiationResponse, BasicSanity) {
    using namespace Davix;

    davix_set_log_level(DAVIX_LOG_ALL);
    S3MultiPartInitiationParser parser;

    int ret = parser.parseChunk(s3_multipart_initiation_response);
    ASSERT_EQ(ret, 0);
    ASSERT_EQ(parser.getUploadId(), "EXAMPLEJZ6e0YupT2h66iePQCc9IEbYbDUy4RTpMeoSMLPRp8Z5o1u8feSRonpvnWsKKG35tI2LB9VDPiCgTy.Gq2VxQLYjrue4Nq.NBdqI-");
}

TEST(XmlSwiftParsing, TestListingDir) {
    using namespace Davix;

    davix_set_log_level(DAVIX_LOG_ALL);
    SwiftPropParser parser;

    int ret = parser.parseChunk(swift_xml_response);
    ASSERT_EQ(ret, 0);

    ASSERT_EQ(3, parser.getProperties().size());

    // verify name
    ASSERT_EQ(std::string("photos/animals/"), parser.getProperties().at(0).filename);
    ASSERT_EQ(std::string("photos/me.jpg"), parser.getProperties().at(1).filename);
    ASSERT_EQ(std::string("photos/plants/"), parser.getProperties().at(2).filename);

    // verify size
    ASSERT_EQ(2906, parser.getProperties().at(1).info.size);
}