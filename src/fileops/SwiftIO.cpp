/*
 * This File is part of Davix, The IO library for HTTP based protocols
 * Copyright (C) CERN 2021
 * Author: Shiting Long <s.long@fz-juelich.de> (Forschungszentrum Juelich)
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

#include "SwiftIO.hpp"
#include <core/ContentProvider.hpp>
#include <utils/davix_logger_internal.hpp>
#include <utils/davix_swift_utils.hpp>


namespace Davix{

static bool is_swift_operation(IOChainContext & context){
    if(context._reqparams->getProtocol() == RequestProtocol::Swift) {
        return true;
    }

    return false;
}

static bool should_use_swift_multipart(IOChainContext & context, dav_size_t size) {
    bool is_swift = is_swift_operation(context);

    if (!is_swift) return false;

    return size > (1024 * 2); // 512 MB
}

static std::string extract_swift_object_path(Uri url) {
    return url.getPath();
}

SwiftIO::SwiftIO() {

}

SwiftIO::~SwiftIO() {

}

static dav_size_t fillBufferWithProviderData(std::vector<char> &buffer, const dav_size_t maxChunkSize, ContentProvider &provider) {
    dav_size_t written = 0u;
    dav_size_t remaining = maxChunkSize;

    while(true) {
        dav_ssize_t bytesRead = provider.pullBytes(buffer.data(), remaining);
        if(bytesRead < 0) {
            throw DavixException(davix_scope_io_buff(), StatusCode::InvalidFileHandle, fmt::format("Error when reading from callback: {}", bytesRead));
        }

        remaining -= bytesRead;
        written += bytesRead;

        if(bytesRead == 0) {
            DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CHAIN, "Reached data provider EOF, received 0 bytes, even though asked for {}", remaining);
            break; // EOF
        }

        if(remaining == 0) {
            DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CHAIN, "Data provider buffer has been filled");
            break; // buffer is full
        }
    }

    DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CHAIN, "Retrieved {} bytes from data provider", written);
    return written;
}

std::string SwiftIO::writeChunk(IOChainContext &iocontext, const char *buff, dav_size_t size, int partNumber) {
    Uri url(iocontext._uri);
    url.setPath(url.getPath() + "/" + std::to_string(partNumber));

    DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CHAIN, "writing chunk #{} with size {}", partNumber, size);

    DavixError * tmp_err=NULL;
    PutRequest req(iocontext._context, url, &tmp_err);
    checkDavixError(&tmp_err);

    req.setParameters(iocontext._reqparams);
    req.setRequestBody(buff, size);
    req.executeRequest(&tmp_err);
    if(!tmp_err && httpcodeIsValid(req.getRequestCode()) == false){
        httpcodeToDavixError(req.getRequestCode(), davix_scope_io_buff(),
                             "write error: ", &tmp_err);
    }
    DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CHAIN, "write result size {}", size);
    checkDavixError(&tmp_err);

    std::string etag;
    if(!req.getAnswerHeader("Etag", etag)) {
        DavixError::setupError(&tmp_err, "Swift::MultiPart", StatusCode::InvalidServerResponse, "Unable to retrieve chunk Etag, necessary when committing chunks");
    }

    DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CHAIN, "chunk #{} written successfully, etag: {}", partNumber, etag);
    return etag;
}

void SwiftIO::commitChunks(IOChainContext & iocontext, const std::vector<Prop> &props) {
    Uri url(iocontext._uri);

    DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CHAIN, "committing {} chunks", props.size());

    std::ostringstream payload;
    payload << "[";
    for(size_t i = 1; i <= props.size(); i++) {
        payload << "{";
        payload << "\"path\":\"" << extract_swift_object_path(url) << "/" << i << "\"}";
        if(i != props.size()) {
            payload << ',';
        }
    }
    payload << "]";
    //std::cout << payload.str() << std::endl;

    DavixError * tmp_err=NULL;
    url.addQueryParam("multipart-manifest", "put");
    PutRequest req(iocontext._context, url, &tmp_err);
    req.addHeaderField("Content-Type", "application/json");
    req.setParameters(iocontext._reqparams);
    req.setRequestBody(payload.str());
    req.executeRequest(&tmp_err);

    if(!tmp_err && httpcodeIsValid(req.getRequestCode()) == false){
        httpcodeToDavixError(req.getRequestCode(), davix_scope_io_buff(),
                             "write error: ", &tmp_err);
    }
    checkDavixError(&tmp_err);
}

dav_ssize_t SwiftIO::writeFromProvider(IOChainContext & iocontext, ContentProvider &provider) {
    if(!should_use_swift_multipart(iocontext, provider.getSize())) {
        CHAIN_FORWARD(writeFromProvider(iocontext, provider));
    }

    DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CHAIN, "Initiating large file upload towards {} to upload file with size {}", iocontext._uri, provider.getSize());

    size_t remaining = provider.getSize();
    const dav_size_t MAX_CHUNK_SIZE = 1024 * 1; // 256 MB

    std::vector<char> buffer;
    buffer.resize(std::min(MAX_CHUNK_SIZE, (dav_size_t) provider.getSize()) + 10);

    std::vector<Prop> props;

    size_t partNumber = 0;

    while(remaining > 0) {
        size_t toRead = std::min( (dav_size_t) provider.getSize(), MAX_CHUNK_SIZE);
        DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CHAIN, "SwiftIO write: toRead from cb {}", toRead);

        dav_size_t bytesRead = fillBufferWithProviderData(buffer, MAX_CHUNK_SIZE, provider);
        if(bytesRead == 0) break; // EOF

        partNumber++;
        props.emplace_back(writeChunk(iocontext, buffer.data(), bytesRead, partNumber), bytesRead);
    }

    commitChunks(iocontext, props);
}

}
