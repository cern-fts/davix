/*
 * This File is part of Davix, The IO library for HTTP based protocols
 * Copyright (C) CERN 2018
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

#include "S3MultiPartInitiationParser.hpp"

namespace Davix {

S3MultiPartInitiationParser::S3MultiPartInitiationParser() : nextIsUploadId(false) {

}

S3MultiPartInitiationParser::~S3MultiPartInitiationParser(){
}


int S3MultiPartInitiationParser::parserStartElemCb(int parent, const char *nspace, const char *name, const char **atts){
    (void) parent;
    (void) nspace;
    (void) atts;

    if(std::string(name) == "UploadId") {
        nextIsUploadId = true;
    }

    return 1;
}

int S3MultiPartInitiationParser::parserCdataCb(int state, const char *cdata, size_t len){
    (void) state;
    (void) len;
    (void) cdata;

    if(nextIsUploadId) {
        uploadId = std::string(cdata, len);
        nextIsUploadId = false;
    }

    return 0;
}

int S3MultiPartInitiationParser::parserEndElemCb(int state, const char *nspace, const char *name){
    (void) state;
    (void) nspace;
    (void) name;
    return 0;
}

std::deque<FileProperties> & S3MultiPartInitiationParser::getProperties(){
    return unusedFileProps;
}

std::string S3MultiPartInitiationParser::getUploadId() const {
    return uploadId;
}

}