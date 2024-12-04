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

#include <davix.hpp>
#include <tools/davix_tool_params.hpp>
#include <tools/davix_tool_util.hpp>


using namespace Davix;
using namespace std;


const std::string scope_main = "Davix::Tools::davix";

std::string  get_base_copy_options(){
    return "  Copy Options:\n"
           "\t--copy-mode:              3rd party copy mode - push(default) or pull\n";
}


static void performanceCallback(const PerformanceData& perfData, void *udata)
{
    (void) udata;
    std::cout << perfData.totalTransferred()
        << " (" << perfData.avgTransfer() << " bytes/sec)"
        << std::endl;
}

static std::string help_msg(const std::string & cmd_path){
    std::string help_str = fmt::format("Usage : {} ", cmd_path);
    help_str += Tool::get_copy_description_options();
    help_str += get_base_copy_options();
    help_str += Tool::get_common_options();

    return help_str;
}


int main(int argc, char** argv) {
    DavixError* tmp_err = NULL;
    Tool::OptParams opts;
    int retcode;

    opts.help_msg = help_msg(argv[0]);

    if ((retcode = parse_davix_copy_options(argc, argv, opts, &tmp_err)) == 0) {
        if (opts.vec_arg.size() != 2) {
            DavixError::setupError(&tmp_err, scope_main, StatusCode::InvalidArgument, "Copy command must contain exactly one <src> and <dst> URL!");
        }

        if (!tmp_err) {
            Uri src(opts.vec_arg[0]);

            if (src.getStatus() != StatusCode::OK) {
                DavixError::setupError(&tmp_err, scope_main, src.getStatus(), "Source \"" + opts.vec_arg[0] + "\" not a valid URL!");
            } else {
                Uri dst(opts.vec_arg[1]);

                if (dst.getStatus() != StatusCode::OK) {
                    DavixError::setupError(&tmp_err, scope_main, dst.getStatus(), "Destination \"" + opts.vec_arg[1] + "\" not a valid URL!");
                }
            }
        }

        if (!tmp_err && (retcode = configureAuth(opts)) == 0) {
            Context c;
            configureContext(c, opts);
            DavixCopy copy(c, &opts.params);
            copy.setPerformanceCallback(performanceCallback, NULL);
            copy.copy(opts.vec_arg[0], opts.vec_arg[1], 1, &tmp_err);
        }
    }

    if (tmp_err) {
        retcode = -1;
    }

    Tool::errorPrint(&tmp_err);
    return retcode;
}
