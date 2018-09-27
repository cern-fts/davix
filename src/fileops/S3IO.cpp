/*
 * This File is part of Davix, The IO library for HTTP based protocols
 * Copyright (C) CERN 2017
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

#include "S3IO.hpp"

namespace Davix{

S3IO::S3IO() {

}

S3IO::~S3IO() {

}

dav_ssize_t S3IO::writeFromFd(IOChainContext & iocontext, int fd, dav_size_t size) {
    CHAIN_FORWARD(writeFromFd(iocontext, fd, size));
}

dav_ssize_t S3IO::writeFromCb(IOChainContext & iocontext, const DataProviderFun & func, dav_size_t size) {
    CHAIN_FORWARD(writeFromCb(iocontext, func, size));
}


}