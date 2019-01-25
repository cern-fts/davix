#!/usr/bin/env bash
set -e

#-------------------------------------------------------------------------------
# Generate a docs folder - run this from the root of the git repository.
#-------------------------------------------------------------------------------
rm -rf build
mkdir build && cd build
cmake -DENABLE_HTML_DOCS=TRUE ..

make sphinx
make doc
