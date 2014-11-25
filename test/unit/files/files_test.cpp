#include <gtest/gtest.h>
#include <ctime>
#include <fileops/davix_reliability_ops.hpp>
#include <davix.hpp>




TEST(MetalinkReplica, HeaderMetalinkParsing){

    std::string header_key, header_value, header_key_location, header_value_torrent, header_value_invalid;
    Davix::Uri origin("http:////lxfsra04a04.cern.ch:80/dpm/cern.ch/home/dteam/group"), origin2("https://google.com/");
    Davix::Uri u;
    int ret = -1;
    header_key = "Link";
    header_key_location = "Location";
    header_value = "<//lxfsra04a04.cern.ch:80/dpm/cern.ch/home/dteam/group?metalink>; rel=describedby; type=\"application/metalink+xml\"";
    header_value_torrent = "<http://example.com/example.ext.torrent>; rel=describedby; type=\"application/x-bittorrent\"; name=\"differentname.ext\"";
    header_value_invalid = "<//lxfsra04a04.cern.ch:80/dpm/cern.ch/home/dteam/group?metalink ; rel=describedby; type=\"application/metalink+xml\" >";

    ret = Davix::davix_metalink_header_parser(header_key, header_value, origin, u);
    ASSERT_EQ(ret,1);
    ASSERT_EQ(u.getStatus(), Davix::StatusCode::OK);
    ASSERT_STREQ("http://lxfsra04a04.cern.ch:80/dpm/cern.ch/home/dteam/group?metalink", u.getString().c_str());

     ret = Davix::davix_metalink_header_parser(header_key, header_value, origin2, u);
     ASSERT_EQ(ret,1);
     ASSERT_EQ(u.getStatus(), Davix::StatusCode::OK);
     ASSERT_STREQ("https://lxfsra04a04.cern.ch:80/dpm/cern.ch/home/dteam/group?metalink", u.getString().c_str());

     ret = Davix::davix_metalink_header_parser(header_key, header_value_torrent, origin, u);
     ASSERT_EQ(ret,0);

     ret = Davix::davix_metalink_header_parser(header_key_location, header_value, origin, u);
     ASSERT_EQ(ret, 0);

     ret = Davix::davix_metalink_header_parser(header_key_location, header_value_invalid, origin, u);
     ASSERT_EQ(ret, 0);
}
