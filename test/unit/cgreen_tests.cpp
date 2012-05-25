/*
 * Copyright (c) Members of the EGEE Collaboration. 2004.
 * See http://www.eu-egee.org/partners/ for details on the copyright holders.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */



/**
 * Unit tests for davix based on the cgreen library
 * @author : Devresse Adrien
 */

#include <cgreen/cgreen.h>
#include <cstdio>
#include <cstdlib>

#include <neon/test_neon.h>
#include <datetime/test_datetime.h>
#include <session/test_session.h>
#include <request/test_request.h>




int main (int argc, char** argv)
{
        //fprintf(stderr, " tests : %s ", getenv("LD_LIBRARY_PATH"));
        TestSuite *global = create_test_suite();
        add_suite(global, datetime_suite());
        add_suite(global, session_suite());
        add_suite(global, neon_suite());
      //  add_suite(global, request_suite()); disable with neon

    if (argc > 1){
        return run_single_test(global, argv[1], create_text_reporter());
    }
        return run_test_suite(global, create_text_reporter());
}


