#!/usr/bin/env bash
set -ex

STAGING_AREA="$1"
TARGET="$2"
SNAPSHOT="$3"

if [[ -z "${SNAPSHOT}" ]]; then
  SNAPSHOT=$(date +%s)
fi

TARGET_OLD="${TARGET}-old-${SNAPSHOT}"

if [[ -d "${TARGET}" ]]; then
  mv -v "${TARGET}" "${TARGET_OLD}"
fi

# Allow EOS FUSE grace time
sleep 10

mv -v "${STAGING_AREA}" "${TARGET}"

if [[ -d "${TARGET_OLD}" ]]; then
  rm -rf "${TARGET_OLD}/"
fi
