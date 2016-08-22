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


#ifndef DAVIX_HOOKS_IMPL_HPP
#define DAVIX_HOOKS_IMPL_HPP

#include <hooks/davix_hooks_fwd.hpp>

// intern file, Never use directly
#ifndef DOXYGEN_SHOULD_SKIP_THIS

namespace Davix{

struct ContextInternal;

#ifdef __DAVIX_HAS_STD_FUNCTION

class HookList{
public:

    RequestPreRunHook _pre_run_req;

    RequestPreSendHook _pre_send_req;

    RequestPreReceHook _pre_rece_req;

private:
    HookList();
    friend struct ContextInternal;
};


// define
template<typename HookType>
inline void hookDefine(HookList & c,  const HookType & hook){
    (void) c;
    (void) hook;
    throw DavixException(std::string("davix::hook"), StatusCode::InvalidHook, "Invalid Hook type");
}

template<>
inline void hookDefine(HookList &c, const RequestPreRunHook & hook){
    c._pre_run_req = hook;
}

template<>
inline void hookDefine(HookList &c, const RequestPreSendHook & hook){
    c._pre_send_req = hook;
}

template<>
inline void hookDefine(HookList &c, const RequestPreReceHook & hook){
    c._pre_rece_req = hook;
}


// get
template<typename HookType>
inline const HookType & hookGet(HookList & c){
    (void) c;
    throw DavixException(std::string("davix::hook"), StatusCode::InvalidHook, "Invalid Hook type");
}

template<>
inline const RequestPreRunHook & hookGet(HookList & c){
    return c._pre_run_req;
}

template<>
inline const RequestPreSendHook & hookGet(HookList & c){
    return c._pre_send_req;
}

template<>
inline const RequestPreReceHook & hookGet(HookList & c){
    return c._pre_rece_req;
}


#endif

}

#endif

#endif // DAVIX_HOOKS_IMPL_HPP
