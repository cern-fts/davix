/*
 * This File is part of Davix, The IO library for HTTP based protocols
 * Copyright (C) CERN
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

#ifndef DAVIX_CONFIG_PARSER_HPP
#define DAVIX_CONFIG_PARSER_HPP

#include <vector>
#include <string>
#include "davix_tool_params.hpp"

namespace Davix {

std::vector<std::string> davix_config_tokenize(const std::string &contents, std::string &err);
bool davix_config_apply(const std::string &filename, Tool::OptParams &params, const std::string &url);
bool davix_config_apply(const std::string &filename, const std::string &contents, const Uri &uri, Tool::OptParams &params);

}

#endif