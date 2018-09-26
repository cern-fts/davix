#!/usr/bin/env bash
set -e

#-------------------------------------------------------------------------------
# Generate a source RPM - run this from the root of the git repository.
#-------------------------------------------------------------------------------
VERSION_FULL=$(./genversion.py --template-string "@VERSION_FULL@")
printf "Version: ${VERSION_FULL}\n"

./packaging/make-dist.sh
TARBALL="davix-${VERSION_FULL}.tar.gz"
BUILD_DIR="$PWD"/build

pushd build
rpmbuild --define "_source_filedigest_algorithm md5" --define "_binary_filedigest_algorithm md5" -ts "${TARBALL}" --define "_topdir ${PWD}" --with server
