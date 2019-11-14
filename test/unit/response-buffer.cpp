/*
 * This File is part of Davix, The IO library for HTTP based protocols
 * Copyright (C) CERN 2019
 * Author: Georgios Bitzes <georgois.bitzes@cern.ch>
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

#include <curl/ResponseBuffer.hpp>
#include <iostream>
#include <gtest/gtest.h>

using namespace Davix;

class Response_Buffer : public testing::TestWithParam<size_t> {};

INSTANTIATE_TEST_CASE_P(Response_BufferTest,
                        Response_Buffer,
                        ::testing::Values(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 16, 31, 32, 63, 64, 77, 256),
                        ::testing::PrintToStringParamName());

TEST_P(Response_Buffer, BasicSanity) {
  std::string tmp = "abc";

  ResponseBuffer buffer(GetParam());
  ASSERT_EQ(buffer.size(), 0u);

  buffer.feed(tmp.c_str() ,3);
  ASSERT_EQ(buffer.size(), 3u);

  tmp = "12345";
  buffer.feed(tmp.c_str(), 5);
  ASSERT_EQ(buffer.size(), 8u);

  tmp = "9";
  buffer.feed(tmp.c_str(), 1);
  ASSERT_EQ(buffer.size(), 9u);

  tmp.resize(6);
  ASSERT_EQ(buffer.consume( (char*) tmp.c_str(), 6u), 6u);
  ASSERT_EQ(tmp, "abc123");
  ASSERT_EQ(buffer.size(), 3u);

  tmp.resize(1);
  ASSERT_EQ(buffer.consume( (char*) tmp.c_str(), 1u), 1u);
  ASSERT_EQ(tmp, "4");
  ASSERT_EQ(buffer.size(), 2u);

  tmp.resize(2);
  ASSERT_EQ(buffer.consume( (char*) tmp.c_str(), 1000u), 2u);
  ASSERT_EQ(tmp, "59");
  ASSERT_EQ(buffer.size(), 0u);

  ASSERT_EQ(buffer.consume( (char*) tmp.c_str(), 1000u), 0u);
  ASSERT_EQ(buffer.consume( (char*) tmp.c_str(), 1000u), 0u);
  ASSERT_EQ(buffer.consume( (char*) tmp.c_str(), 1000u), 0u);

  tmp = "777";
  buffer.feed(tmp.c_str(), 3);
  ASSERT_EQ(buffer.size(), 3u);

  tmp.resize(2);
  ASSERT_EQ(buffer.consume( (char*) tmp.c_str(), 2u), 2u);
  ASSERT_EQ(tmp, "77");
  ASSERT_EQ(buffer.size(), 1u);

  tmp = "00000";
  buffer.feed(tmp.c_str(), 5);
  ASSERT_EQ(buffer.size(), 6u);

  tmp.resize(6);
  ASSERT_EQ(buffer.consume( (char*) tmp.c_str(), 600u), 6u);
  ASSERT_EQ(tmp, "700000");
  ASSERT_EQ(buffer.size(), 0u);
}

TEST_P(Response_Buffer, AlternateFeedAndConsume) {
  std::string contents = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Mauris porttitor urna in diam ultricies semper. Vivamus gravida purus eu erat condimentum, ullamcorper aliquam dui commodo. Fusce id nunc euismod mauris venenatis cursus non vel odio. Aliquam porttitor urna eget nibh cursus, eget ultricies quam sagittis. Donec pulvinar fermentum nunc, id rhoncus justo convallis sed. Donec suscipit quis lectus eget maximus. Etiam ut pharetra odio. Morbi ac nulla rhoncus, placerat quam varius, ultrices justo.";

  ResponseBuffer buffer(GetParam());
  ASSERT_EQ(buffer.size(), 0u);

  std::string reconstructed;
  reconstructed.resize(contents.size());

  size_t fed = 0;
  size_t consumed = 0;

  while(true) {
    size_t stride = (rand() % 10) + 0;

    if(stride + fed > contents.size()) {
      break;
    }

    buffer.feed(contents.c_str()+fed, stride);
    fed += stride;

    stride = (rand() % 10) + 0;
    std::string target;
    target.resize(stride);

    stride = buffer.consume( (char*) target.c_str(), stride);
    ::memcpy( (char*) (reconstructed.c_str()+consumed), target.c_str(), stride);
    consumed += stride;
  }

  // Feed the rest
  buffer.feed(contents.c_str()+fed, contents.size()-fed);

  std::string target;
  target.resize(contents.size());

  size_t stride = buffer.consume( (char*) target.c_str(), contents.size());
  ::memcpy( (char*) (reconstructed.c_str()+consumed), target.c_str(), stride);

  ASSERT_EQ(reconstructed.size(), contents.size());
  ASSERT_EQ(reconstructed, contents);
  ASSERT_EQ(buffer.size(), 0u);
}

TEST_P(Response_Buffer, SeparateFeedAndConsume) {
  std::string contents = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Mauris porttitor urna in diam ultricies semper. Vivamus gravida purus eu erat condimentum, ullamcorper aliquam dui commodo. Fusce id nunc euismod mauris venenatis cursus non vel odio. Aliquam porttitor urna eget nibh cursus, eget ultricies quam sagittis. Donec pulvinar fermentum nunc, id rhoncus justo convallis sed. Donec suscipit quis lectus eget maximus. Etiam ut pharetra odio. Morbi ac nulla rhoncus, placerat quam varius, ultrices justo.";

  ResponseBuffer buffer(GetParam());
  ASSERT_EQ(buffer.size(), 0u);

  size_t fed = 0;
  while(true) {
    size_t stride = (rand() % 10) + 0;

    if(stride + fed > contents.size()) {
      break;
    }

    buffer.feed(contents.c_str()+fed, stride);
    fed += stride;
  }

  // Feed the rest
  buffer.feed(contents.c_str()+fed, contents.size()-fed);

  std::string reconstructed;
  reconstructed.resize(contents.size());

  size_t consumed = 0;
  while(true) {
    size_t stride = (rand() % 10) + 0;
    size_t output = buffer.consume( (char*) reconstructed.c_str()+consumed, stride);
    consumed += output;
    if(stride != 0 && output == 0) break;
  }

  ASSERT_EQ(contents.size(), consumed);
  ASSERT_EQ(contents, reconstructed);
}
