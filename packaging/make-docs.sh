#!/usr/bin/env bash
set -e

#-------------------------------------------------------------------------------
# Make SRPM to get a list of build dependencies, this is ugly, fix.
#-------------------------------------------------------------------------------
git submodule update --init --recursive
./packaging/make-srpm.sh
sudo yum-builddep -y build/SRPMS/*

#-------------------------------------------------------------------------------
# Generate a docs folder - run this from the root of the git repository.
#-------------------------------------------------------------------------------
rm -rf build
mkdir build && cd build
cmake -DENABLE_HTML_DOCS=TRUE ..

make sphinx
make doc
