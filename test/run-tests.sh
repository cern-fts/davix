#!/usr/bin/env bash
set -e

# Please note: You need to have a credentials directory at the top level
# of the repository in order to run the functional tests.
#
# The credentials directory must contain a valid VOMS proxy certificate, AWS as well
# as Azure credentials.
#
# Please contact the maintainer if you need advice on how to run the tests.
#
# You need to run this script while being at the top level of the repository,
# and not in $REPO/tests

CORES=$(grep -c ^processor /proc/cpuinfo)
git submodule update --recursive --init

credentials/obtain-proxy.sh

rm -rf build
mkdir build
cd build
cmake -DENABLE_THIRD_PARTY_COPY=TRUE ..
make -j $CORES
# make abi-check
(ctest --no-compress-output -T Test || true)
