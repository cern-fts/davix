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

#include <gtest/gtest.h>
#include "../drunk-server/DrunkServer.hpp"
#include "../drunk-server/ConnectionInitiator.hpp"

TEST(DrunkServer, NoClients) {
  DrunkServer ds(22222);
  std::unique_ptr<DrunkServer::Connection> conn = ds.accept(1);
  ASSERT_FALSE(conn);
}

TEST(DrunkServer, ClientInteraction) {
  DrunkServer ds(22222);

  ConnectionInitiator initiator("localhost", 22222);
  std::unique_ptr<DrunkServer::Connection> connSrv = ds.accept(1);
  ASSERT_TRUE(connSrv);

  std::unique_ptr<DrunkServer::Connection> connCl(new DrunkServer::Connection(initiator.getFd()));

  ASSERT_EQ(connCl->write("hey there"), 9);

  std::string buff;
  ASSERT_EQ(connSrv->read(buff, 9), 9);
  ASSERT_EQ(buff, "hey there");
}

