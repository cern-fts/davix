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

#ifndef DAVIX_OP_HPP
#define DAVIX_OP_HPP

#include <davix.hpp>
#include <tools/davix_tool_params.hpp>
#include <tools/davix_tool_util.hpp>
#include <tools/davix_mutex.hpp>

namespace Davix{

class DavixOp{

protected:
    std::string _target_url;
    std::string _destination_url;
    std::string opType;
    Tool::OptParams _opts;
    Context& _c;
    std::string _scope;

    DavixOp(const Tool::OptParams& opts, std::string target_url, std::string destination_url, Context& c);

    //virtual ~DavixOp();

public:
    virtual int executeOp()=0;
    virtual ~DavixOp();
    std::string getTargetUrl();
    std::string getDestinationUrl();
    std::string getOpType();

private:

};


class GetOp : public DavixOp{

public:
    GetOp(const Tool::OptParams& opts, std::string target_url, std::string destination_url, Context& c);
    virtual ~GetOp();
    virtual int executeOp();
    int getOutFd();

private:

};


class PutOp : public DavixOp{

public:
    PutOp(const Tool::OptParams& opts, std::string target_url, std::string destination_url, dav_size_t file_size, Context& c);
    virtual ~PutOp();
    virtual int executeOp();
    int getInFd(DavixError** err);

private:
    dav_size_t _file_size;


};


class DeleteOp : public DavixOp{

public:
    // for s3 batch delete, need post request and body
    DeleteOp(const Tool::OptParams& opts, std::string destination_url, Context& c, std::string buf);
    // for anything else
    DeleteOp(const Tool::OptParams& opts, std::string destination_url, Context& c);
    virtual ~DeleteOp();
    virtual int executeOp();
    std::string calculateMD5(std::string content, DavixError** err);
    void parse_deletion_result(int code, const Uri & u, const std::string & scope, const std::vector<char> & body);

private:
    std::string _buf;


};

// forward declaration
class DavixTaskQueue;

// ListOp, used in davix-ls for recursively crawling the namespace
class ListOp : public DavixOp{

public:
    ListOp(const Tool::OptParams& opts, std::string target_url, Context& c, DavixTaskQueue* listing_tq, FILE* filestream, pthread_mutex_t& output_mutex);
    virtual ~ListOp();
    virtual int executeOp();

private:
    DavixTaskQueue* _listing_tq;
    FILE* _filestream;
    pthread_mutex_t& _output_mutex;
    static void display_file_entry(const std::string & filename, const Tool::OptParams & opts, FILE* filestream);
    static void display_long_file_entry(const std::string & filename,  struct stat* st, const Tool::OptParams & opts, FILE* filestream);
};


// ListppOp, used in davix-get for recursively crawling namespace and getting files
// different than ListOp, doesn't print out info of entries, instead, it populates another taskqueue with GetOps
class ListppOp : public DavixOp{

public:
    ListppOp(const Tool::OptParams& opts, std::string target_url, std::string destination_url, Context& c, DavixTaskQueue* tq, DavixTaskQueue* listing_tq);
    virtual ~ListppOp();
    virtual int executeOp();

private:
    DavixTaskQueue* _tq;
    DavixTaskQueue* _listing_tq;
};

}

#endif // DAVIX_OP_HPP
