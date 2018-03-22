#!/usr/bin/env bash
set -e

AUTHOR_NAME=${1}
AUTHOR_EMAIL=${2}

VERSION_FULL=$(./genversion.py --template-string "@VERSION_FULL@")

if grep --quiet --fixed-strings "davix (${VERSION_FULL}" packaging/debian/changelog; then
  printf "Changelog does not need patching.\n"
else
  line1="davix (${VERSION_FULL}-1) unstable; urgency=low"
  line2="\n  * Update to version ${VERSION_FULL}"
  line3="\n -- ${AUTHOR_NAME} <${AUTHOR_EMAIL}>  $(date -R)\n"
  sed -i "1i $line1\n$line2\n$line3" packaging/debian/changelog

  printf "Changelog patched!\n"
fi
