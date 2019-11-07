#!/usr/bin/env bash

# An important user of davix (ROOT project) does not want to depend on librt.
# This means we can only use clock_gettime on CC7, not SLC6.
#
# (clock_gettime was moved from librt onto glibc proper, starting from CC7)
#
# This script patches out libcurl support for clock_gettime entirely when building on SLC6.

set -x

if [ -f lib/curl_config.h ]; then
  echo '#include<time.h>
        int main() { struct timespec ts; clock_gettime(CLOCK_MONOTONIC, &ts); return 0; }' &> test.c
  gcc test.c -o /dev/null

  if [ $? -ne 0 ]; then
    grep "HAVE_CLOCK_GETTIME" lib/curl_config.h
    sed -i 's|^#define HAVE_CLOCK_GETTIME_MONOTONIC 1|// #define HAVE_CLOCK_GETTIME_MONOTONIC 1|g' lib/curl_config.h
    grep "HAVE_CLOCK_GETTIME" lib/curl_config.h
  fi

  rm test.c
fi

