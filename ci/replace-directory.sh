#!/usr/bin/env bash
set -e

STAGING_AREA="$1"
TARGET="$2"
OLD_CONTENTS="${TARGET}-old"

test ! -d "$TARGET" || mv "$TARGET" "$OLD_CONTENTS"

sleep 30
mv "$STAGING_AREA" "$TARGET"
test ! -d "$OLD_CONTENTS" || rm -rf "$OLD_CONTENTS"

