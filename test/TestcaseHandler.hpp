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

#ifndef DAVIX_TEST_TESTCASE_HANDLER_HPP
#define DAVIX_TEST_TESTCASE_HANDLER_HPP

#include <string>
#include <vector>
#include <memory>
#include <functional>

namespace Davix {
  class DavixError;
}

class TermFormat {
public:
  static std::string blue(const std::string &msg);
  static std::string green(const std::string &msg);
  static std::string purple(const std::string &msg);
  static std::string red(const std::string &msg);
};

class TestcaseHandler {
public:
  //----------------------------------------------------------------------------
  // Constructor
  //----------------------------------------------------------------------------
  TestcaseHandler(TestcaseHandler *parent = NULL);

  //----------------------------------------------------------------------------
  // Set name
  //----------------------------------------------------------------------------
  void setName(const std::string &name);

  //----------------------------------------------------------------------------
  // Log info
  //----------------------------------------------------------------------------
  void info(const std::string &msg);

  //----------------------------------------------------------------------------
  // Check condition
  //----------------------------------------------------------------------------
  void check(bool check, const std::string &msg);

  //----------------------------------------------------------------------------
  // Log successful check
  //----------------------------------------------------------------------------
  void pass(const std::string &msg);

  //----------------------------------------------------------------------------
  // Log error, fail the test
  //----------------------------------------------------------------------------
  void fail(const std::string &msg);

  //----------------------------------------------------------------------------
  // Get nest level
  //----------------------------------------------------------------------------
  size_t getNestLevel() const;

  //----------------------------------------------------------------------------
  // Test ok so far?
  //----------------------------------------------------------------------------
  bool ok() const;

  //----------------------------------------------------------------------------
  // Ensure no davix error occurred
  //----------------------------------------------------------------------------
  bool checkDavixError(Davix::DavixError **err);

  //----------------------------------------------------------------------------
  // Make child
  //----------------------------------------------------------------------------
  TestcaseHandler& makeChild();

private:
  std::string mTestcaseName;
  TestcaseHandler *mParent;

  bool mFailed;
  size_t mLevel;

  std::string mLinePrefix;
  std::string mLinePrefixLog;

  std::vector<std::unique_ptr<TestcaseHandler>> mChildren;
};

#endif

