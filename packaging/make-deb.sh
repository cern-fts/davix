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

#-------------------------------------------------------------------------------
# Patch debian changelog, but only if this is a CI build. Otherwise, for stable
# releases, the changelog has been updated already by the release script.
#-------------------------------------------------------------------------------
MINIPATCH=$(./genversion.py --template-string "@VERSION_MINIPATCH@")

if [[ ! -z "$MINIPATCH" ]]; then
  pushd "${BUILD_ARENA}/davix-${VERSION_FULL}/debian"

  CURRENT_DATE=$(date -R)
  echo "davix (${VERSION_FULL}-1) unstable; urgency=low"           >> patched-changelog
  echo ""                                                          >> patched-changelog
  echo "  * CI build, update to version ${VERSION_FULL}"           >> patched-changelog
  echo ""                                                          >> patched-changelog
  echo " -- CI Robot <davix.tests at cern.ch>  ${CURRENT_DATE}"    >> patched-changelog
  echo ""                                                          >> patched-changelog

  cat patched-changelog changelog > tmp
  mv tmp changelog
  rm patched-changelog

  popd
fi

cp "${BUILD_DIR}/${TARBALL}" "${BUILD_ARENA}/davix_${VERSION_FULL}.orig.tar.gz"

#-------------------------------------------------------------------------------
# Build packages
#-------------------------------------------------------------------------------
pushd "${BUILD_ARENA}/davix-${VERSION_FULL}"
debuild -us -uc
