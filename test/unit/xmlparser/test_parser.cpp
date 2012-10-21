
#include <ctime>
#include <datetime/datetime_utils.h>
#include <davix.h>
#include <xmlpp/davxmlparser.h>
#include <gtest/gtest.h>

TEST(XmlParserInstance, createParser){
    Davix::DavXMLParser * parser = new Davix::DavXMLParser();
    ASSERT_EQ(NULL,parser->getLastErr());
    delete parser;
}

