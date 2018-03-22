#!/usr/bin/env bash
set -o nounset
set -o errexit

#-------------------------------------------------------------------------------
# Generate debian packages - run this from the root of the git repository.
#-------------------------------------------------------------------------------

#-------------------------------------------------------------------------------
# Fetch version.
#-------------------------------------------------------------------------------
VERSION_FULL=$(./genversion.py --template-string "@VERSION_FULL@")
printf "Version: ${VERSION_FULL}\n"

#-------------------------------------------------------------------------------
# Find build path.
#-------------------------------------------------------------------------------
BUILD_DIR="${PWD}/build"

#-------------------------------------------------------------------------------
# Make release tarball.
#-------------------------------------------------------------------------------
./packaging/make-dist.sh
TARBALL="davix-${VERSION_FULL}.tar.gz"

#-------------------------------------------------------------------------------
# Prepare debian build arena.
#-------------------------------------------------------------------------------
BUILD_ARENA="${BUILD_DIR}/debian-build-arena"
rm -rf "${BUILD_ARENA}"
mkdir -p "${BUILD_ARENA}"
tar xf "${BUILD_DIR}/${TARBALL}" -C "${BUILD_ARENA}"

cp -r packaging/debian "${BUILD_ARENA}/davix-${VERSION_FULL}"
cp "${BUILD_DIR}/${TARBALL}" "${BUILD_ARENA}/davix_${VERSION_FULL}.orig.tar.gz"

#-------------------------------------------------------------------------------
# Build dsc
#-------------------------------------------------------------------------------
pushd "${BUILD_ARENA}/davix-${VERSION_FULL}"
mk-build-deps --install "${BUILD_DIR}/../packaging/debian/control" --remove
debuild -us -uc -S

#-------------------------------------------------------------------------------
# Build debian packages
#-------------------------------------------------------------------------------
popd
pushd "${BUILD_ARENA}"
pbuilder build --buildresult . *.dsc
