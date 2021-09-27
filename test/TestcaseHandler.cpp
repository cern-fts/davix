/*
 * This File is part of Davix, The IO library for HTTP based protocols
 * Copyright (C) CERN 2019
 * Author: Georgios Bitzes <georgios.bitzes@cern.ch>
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

#include "TestcaseHandler.hpp"
#include <iostream>
#include <sstream>
#include <davix.hpp>

#define SSTR(message) static_cast<std::ostringstream&>(std::ostringstream().flush() << message).str()

//------------------------------------------------------------------------------
// TermFormat: red
//------------------------------------------------------------------------------
std::string TermFormat::red(const std::string &msg) {
  std::ostringstream ss;
  ss << "\033[91;1m" << msg << "\033[0m";
  return ss.str();
}

//------------------------------------------------------------------------------
// TermFormat: green
//------------------------------------------------------------------------------
std::string TermFormat::green(const std::string &msg) {
  std::ostringstream ss;
  ss << "\033[92;1m" << msg << "\033[0m";
  return ss.str();
}

//------------------------------------------------------------------------------
// TermFormat: Blue
//------------------------------------------------------------------------------
std::string TermFormat::blue(const std::string &msg) {
  std::ostringstream ss;
  ss << "\033[94;1m" << msg << "\033[0m";
  return ss.str();
}

//------------------------------------------------------------------------------
// TermFormat: purple
//------------------------------------------------------------------------------
std::string TermFormat::purple(const std::string &msg) {
  std::ostringstream ss;
  ss << "\033[95;1m" << msg << "\033[0m";
  return ss.str();
}

//------------------------------------------------------------------------------
// Split utility function - move to common header eventually
//------------------------------------------------------------------------------
static std::vector<std::string> split(std::string data, std::string token) {
  std::vector<std::string> output;
  size_t pos = std::string::npos;
  do {
    pos = data.find(token);
    output.push_back(data.substr(0, pos));
    if(std::string::npos != pos) data = data.substr(pos + token.size());
  } while (std::string::npos != pos);
  return output;
}

//------------------------------------------------------------------------------
// Multiply string
//------------------------------------------------------------------------------
static std::string multiplyString(const std::string &s, size_t times) {
  std::ostringstream ss;
  for(size_t i = 0; i < times; i++) {
    ss << s;
  }

  return ss.str();
}

//------------------------------------------------------------------------------
// Constructor
//------------------------------------------------------------------------------
TestcaseHandler::TestcaseHandler(TestcaseHandler *parent)
: mParent(parent), mFailed(false) {

  if(mParent) {
    mLevel = mParent->getNestLevel() + 1;
  }
  else {
    mLevel = 0;
  }

  mLinePrefix = multiplyString(" ", mLevel * 4);
  mLinePrefixLog = multiplyString(" ", (mLevel+1) * 4);
}

//------------------------------------------------------------------------------
// Set name
//------------------------------------------------------------------------------
void TestcaseHandler::setName(const std::string &name) {
  mTestcaseName = name;
  std::cout << mLinePrefix << TermFormat::blue("[TEST] ") << name << std::endl;
}

//------------------------------------------------------------------------------
// Log info
//------------------------------------------------------------------------------
void TestcaseHandler::info(const std::string &msg) {
  std::vector<std::string> parts = split(msg, "\n");

  std::ostringstream ss;
  for(size_t i = 0; i < parts.size(); i++) {
    ss << mLinePrefixLog << parts[i] << std::endl;
  }

  std::cout << ss.str();
}

//------------------------------------------------------------------------------
// Log successful check
//------------------------------------------------------------------------------
void TestcaseHandler::pass(const std::string &msg) {
  std::cout << mLinePrefixLog << TermFormat::green("[OK] ") << msg << std::endl;
}

//------------------------------------------------------------------------------
// Log error, fail the test
//------------------------------------------------------------------------------
void TestcaseHandler::fail(const std::string &msg) {
  mFailed = true;

  std::vector<std::string> parts = split(msg, "\n");

  std::ostringstream ss;
  for(size_t i = 0; i < parts.size(); i++) {
    ss << mLinePrefixLog << TermFormat::red("[FAIL] ") << parts[i] << std::endl;
  }

  std::cout << ss.str();
}

//------------------------------------------------------------------------------
// Check condition
//------------------------------------------------------------------------------
void TestcaseHandler::check(bool test, const std::string &msg) {
  if(test) {
    pass(msg);
  }
  else {
    fail(msg);
  }
}

//------------------------------------------------------------------------------
// Get nest level
//------------------------------------------------------------------------------
size_t TestcaseHandler::getNestLevel() const {
  return mLevel;
}

//------------------------------------------------------------------------------
// Test ok so far?
//------------------------------------------------------------------------------
bool TestcaseHandler::ok() const {
  if(mFailed) {
    return false;
  }

  for(size_t i = 0; i < mChildren.size(); i++) {
    if(!mChildren[i]->ok()) {
      return false;
    }
  }

  return true;
}

//------------------------------------------------------------------------------
// Make child
//------------------------------------------------------------------------------
TestcaseHandler& TestcaseHandler::makeChild() {
  mChildren.emplace_back(new TestcaseHandler(this));
  return *(mChildren.back().get());
}

//------------------------------------------------------------------------------
// Ensure no davix error occurred
//------------------------------------------------------------------------------
bool TestcaseHandler::checkDavixError(Davix::DavixError **err) {
  if(err && *err) {
    this->fail(SSTR("DavixError: " << (*err)->getErrMsg()));
    return false;
  }

  return true;
}
