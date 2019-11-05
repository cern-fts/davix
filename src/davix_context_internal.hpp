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

#ifndef DAVIX_CONTEXTINTERNAL_H
#define DAVIX_CONTEXTINTERNAL_H

#include <davixcontext.hpp>
#include <neon/neonsessionfactory.hpp>
#include <utils/davix_logger_internal.hpp>

#include <dlfcn.h>


namespace Davix {

/// @cond HIDDEN_SYMBOLS

class RedirectionResolver;
class SessionFactory;


struct ContextExplorer{

static SessionFactory & SessionFactoryFromContext(Context & c);
static RedirectionResolver & RedirectionResolverFromContext(Context &c);

};


// libpath handler
struct LibPath{
    LibPath();

    std::string path;

};


const std::string & getLibPath();




///@endcond

} // namespace Davix

#endif // DAVIX_CONTEXTINTERNAL_H
