#!/usr/bin/env bash

#-------------------------------------------------------------------------------
# This script tries to improve robustness when regenerating a repo
# which lives on EOS.
#-------------------------------------------------------------------------------

if [ -z "$1" ]; then
  echo "syntax: $0 <path to repo>"
  exit 1
fi

REPO="$1"
createrepo -v "${REPO}" --update --workers 1

if [[ $? != 0 ]]; then
  exit $?
fi

# Force to access the file, or the size will be right, but the file empty
cat "${REPO}/repodata/repomd.xml" > /dev/null
# Stat the repomd.xml
REPOSIZE=$(stat --printf "%s" "${REPO}/repodata/repomd.xml")
# If 0, the bug is still there, so move ourselves from .repodata to the final destination
if [[ $REPOSIZE == 0 ]]; then
    echo "Trigger workaround for EOS"
    mv "${REPO}/.repodata/repomd.xml" "${REPO}/repodata/repomd.xml"
fi
