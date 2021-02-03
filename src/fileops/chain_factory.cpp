/*
 * This File is part of Davix, The IO library for HTTP based protocols
 * Copyright (C) CERN 2013
 * Author: Adrien Devresse <adrien.devresse@cern.ch>
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

#include "chain_factory.hpp"

#include "davmeta.hpp"
#include "httpiovec.hpp"
#include "davix_reliability_ops.hpp"
#include "iobuffmap.hpp"
#include "AzureIO.hpp"
#include "S3IO.hpp"
#include "SwiftIO.hpp"

namespace Davix{


ChainFactory::ChainFactory(){}


HttpIOChain& ChainFactory::instanceChain(const CreationFlags & flags, HttpIOChain & c){
    HttpIOChain* elem;
    elem= c.add(new MetalinkOps())->add(new AutoRetryOps())->add(new S3MetaOps())->add(new SwiftMetaOps())->add(new AzureMetaOps())->add(new HttpMetaOps());

    // add posix to the chain if needed
    if(flags[CHAIN_POSIX] == true){
        elem = elem->add(new HttpIOBuffer());
    }

    elem->add(new S3IO())->add(new SwiftIO())->add(new AzureIO())->add(new HttpIO())->add(new HttpIOVecOps());
    return c;
}

}
