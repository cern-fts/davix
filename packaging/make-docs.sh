#!/usr/bin/env bash
set -e

#-------------------------------------------------------------------------------
# Make SRPM to get a list of build dependencies
#-------------------------------------------------------------------------------
git submodule update --init --recursive
./packaging/make-srpm.sh
dnf builddep -y build/SRPMS/*

#-------------------------------------------------------------------------------
# Generate a docs folder - run this from the root of the git repository.
#-------------------------------------------------------------------------------
rm -rf build
mkdir build && cd build
sphinx-build-3 -q -b html "${PWD}/sphinx" "${PWD}/build/html/"

cmake -DENABLE_HTML_DOCS=TRUE ..
make doc
