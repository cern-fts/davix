/*
 * This File is part of Davix, The IO library for HTTP based protocols
 * Copyright (C) CERN 2013
 * Author: Kwong Tat Cheung <kwong.tat.cheung@cern.ch>
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

#ifndef DAVIX_MUTEX_HPP
#define DAVIX_MUTEX_HPP

#include <pthread.h>

// Simple wrapper for scoped mutex

namespace Davix{

class DavixMutex{

public:
    DavixMutex(pthread_mutex_t& mutex) : _mutex(mutex)
    {
        pthread_mutex_lock(&_mutex);
    }

    ~DavixMutex()
    {
        pthread_mutex_unlock(&_mutex);
    }

    void lock()
    {
        pthread_mutex_lock(&_mutex);
    }

    void unlock()
    {
        pthread_mutex_unlock(&_mutex);
    }

    pthread_mutex_t* getMutex()
    {
        return &_mutex;
    }

private:
    pthread_mutex_t& _mutex;
};

}

#endif // DAVIX_MUTEX_HPP
