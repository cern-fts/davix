/*
 * This File is part of Davix, The IO library for HTTP based protocols
 * Copyright (C) CERN 2026
 * Author: Nitesh Kumar <niteshg7562@gmail.com>
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 */

#include <gtest/gtest.h>
#include <davix.hpp>

#include "../drunk-server/DrunkServer.hpp"
#include "../drunk-server/Interactors.hpp"
#include "../drunk-server/LineReader.hpp"

#include <cstdlib>
#include <string>

using namespace Davix;

namespace {

const char propfind_response[] =
    "<?xml version=\"1.0\"?>"
    "<D:multistatus xmlns:D=\"DAV:\">"
    "<D:response><D:href>/</D:href><D:propstat><D:prop>"
    "<D:displayname>root</D:displayname>"
    "<D:getcontentlength>0</D:getcontentlength>"
    "<D:resourcetype><D:collection/></D:resourcetype>"
    "</D:prop><D:status>HTTP/1.1 200 OK</D:status></D:propstat></D:response>"
    "<D:response><D:href>/child</D:href><D:propstat><D:prop>"
    "<D:displayname>child</D:displayname>"
    "<D:getcontentlength>4</D:getcontentlength>"
    "<D:resourcetype/>"
    "</D:prop><D:status>HTTP/1.1 200 OK</D:status></D:propstat></D:response>"
    "</D:multistatus>";

class PropfindInteractor : public BasicInteractor {
public:
    explicit PropfindInteractor(const std::string &expected_content_type)
        : _expected_content_type(expected_content_type) {}

    void main(ThreadAssistant &assistant) override {
        std::string content_type;
        size_t content_type_count = 0;
        size_t content_length = 0;

        while (true) {
            const std::string line = consumeLine();

            if (line.empty()) {
                return;
            }

            if (line == "\r\n") {
                break;
            }

            if (line.compare(0, 14, "Content-Type: ") == 0) {
                content_type = line.substr(14, line.size() - 16);
                ++content_type_count;
            } else if (line.compare(0, 16, "Content-Length: ") == 0) {
                content_length =
                    std::strtoul(line.substr(16).c_str(), nullptr, 10);
            }
        }

        std::string request_body;

        while (request_body.size() < content_length) {
            std::string chunk;
            const ssize_t read =
                _conn->read(chunk, content_length - request_body.size());

            if (read <= 0) {
                return;
            }

            request_body += chunk;
        }

        if (content_type_count != 1 ||
            content_type != _expected_content_type ||
            request_body.find("<D:propfind") == std::string::npos) {
            const std::string response =
                "HTTP/1.1 415 Unsupported Media Type\r\n"
                "Content-Length: 0\r\n"
                "\r\n";

            _conn->write(response);
            return;
        }

        const std::string body(propfind_response);
        const std::string response =
            "HTTP/1.1 207 Multi-Status\r\n"
            "Content-Type: application/xml\r\n"
            "Content-Length: " +
            std::to_string(body.size()) +
            "\r\n\r\n" +
            body;

        if (_conn->write(response) == static_cast<ssize_t>(response.size())) {
            _is_ok = true;
        }
    }

private:
    std::string _expected_content_type;
};

class WebdavPropfindTest : public ::testing::Test {
protected:
    void runListing(bool use_curl, const std::string &content_type) {
        if (use_curl) {
            setenv("DAVIX_USE_LIBCURL", "true", 1);
        } else {
            unsetenv("DAVIX_USE_LIBCURL");
        }

        DrunkServer server(22222);
        PropfindInteractor interactor(content_type);
        server.autoAcceptNext(&interactor);

        Context context;
        RequestParams params;

        if (!content_type.empty()) {
            params.addHeader("Content-Type", content_type);
        }

        DavPosix posix(&context);
        DavixError *error = nullptr;

        DAVIX_DIR *directory =
            posix.opendir(&params, "dav://localhost:22222/", &error);

        ASSERT_NE(directory, nullptr);
        ASSERT_EQ(error, nullptr);

        struct dirent *entry = posix.readdir(directory, &error);

        ASSERT_NE(entry, nullptr);
        ASSERT_STREQ(entry->d_name, "child");
        ASSERT_EQ(error, nullptr);

        ASSERT_EQ(posix.closedir(directory, &error), 0);
        ASSERT_EQ(error, nullptr);
        ASSERT_TRUE(interactor.ok());
    }
};

TEST_F(WebdavPropfindTest, AddsContentTypeForNeon) {
    runListing(false, "application/xml");
}

TEST_F(WebdavPropfindTest, AddsContentTypeForCurl) {
    runListing(true, "application/xml");
}

TEST_F(WebdavPropfindTest, PreservesCustomContentType) {
    runListing(false, "application/custom+xml");
}

TEST_F(WebdavPropfindTest, DoesNotAddContentTypeToGet) {
    unsetenv("DAVIX_USE_LIBCURL");

    DrunkServer server(22222);

    SingleShotInteractor interactor(
        std::string("GET / HTTP/1.1\r\nUser-Agent: ") +
            RequestParams().getUserAgent() +
            " neon/0.0.29\r\n"
            "Keep-Alive: \r\n"
            "Connection: Keep-Alive\r\n"
            "TE: trailers\r\n"
            "Host: localhost:22222\r\n"
            "\r\n",
        "HTTP/1.1 200 OK\r\n"
        "Content-Length: 0\r\n"
        "\r\n");

    server.autoAcceptNext(&interactor);

    Context context;
    DavixError *error = nullptr;

    HttpRequest request(
        context,
        "http://localhost:22222/",
        &error);

    ASSERT_EQ(error, nullptr);
    ASSERT_EQ(request.executeRequest(&error), 0);
    ASSERT_EQ(error, nullptr);
    ASSERT_TRUE(interactor.ok());
}

}
