#!/usr/bin/env bash
set -e

# Ensure "epel-release" package is installed
if ! rpm -q --quiet epel-release ; then
  dnf install -y epel-release || true
fi

# Fedora rawhide (FC41)
dnf install -y dnf5-plugins || true

dnf install -y dnf-plugins-core git rpm-build tree which \
               python3 cmake make gcc gcc-c++
