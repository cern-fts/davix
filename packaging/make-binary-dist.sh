#!/usr/bin/env bash
set -e

#-------------------------------------------------------------------------------
# Description:
# Generate a binary tarball for the Davix project.
# Binary tarballs are a collection of binaries and libraries,
# compiled without debug symbols and no install prefix.
# They are usually deployed on software distribution systems, such as CVMFS.
#
# Note: run this from the root of the git repository
#-------------------------------------------------------------------------------

#-------------------------------------------------------------------------------
# Generate variables for binary distribution
#-------------------------------------------------------------------------------

VERSION=$(./genversion.py --template-string "@VERSION_FULL@")
DIST=$(rpm --eval "%{?dist}" | cut -d. -f2)
ARCH=$(rpm --eval "%{_arch}")
FILENAME="davix-${VERSION}-${DIST}.${ARCH}"

printf "====================\n"
printf "Version: %s\n" "${VERSION}"
printf "Dist: %s\n" "${DIST}"
printf "Arch: %s\n" "${ARCH}"
printf "Filename: %s\n" "${FILENAME}"
printf "====================\n"

#-------------------------------------------------------------------------------
# Install all build dependencies
#-------------------------------------------------------------------------------

rm -rf build/ binary-tarball/
git submodule update --recursive --init
./packaging/make-srpm.sh

if [[ -f /usr/bin/dnf ]]; then
  dnf builddep -y build/SRPMS/davix-*.src.rpm
else
  yum-builddep -y build/SRPMS/davix-*.src.rpm
fi

#-------------------------------------------------------------------------------
# Compile davix and create binary tarball
#-------------------------------------------------------------------------------

rm -rf build/
mkdir build/ binary-tarball/
TARBALL_DIR="${PWD}/binary-tarball/"

pushd build/
cmake3 ../ -DCMAKE_INSTALL_PREFIX="/" -DCMAKE_BUILD_TYPE=Release -DENABLE_THIRD_PARTY_COPY=True \
           -DENABLE_HTML_DOCS=TRUE -DDOC_INSTALL_DIR="/share/doc/davix-${VERSION}" -Wno-dev
make
make doc
make DESTDIR="${TARBALL_DIR}" install
find "${TARBALL_DIR}" -type f -regex ".*\(1\|3\)$" -exec gzip {} \;
popd

tar -pczf "${FILENAME}.tar.gz" "${TARBALL_DIR}"
printf "Wrote: %s.tar.gz\n" "${FILENAME}"
