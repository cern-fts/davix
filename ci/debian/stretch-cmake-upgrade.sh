#!/usr/bin/env bash
set -e

# Add stretch-backport releases
# This is needed to install a newer version of cmake
echo "deb http://deb.debian.org/debian stretch-backports main" >> /etc/apt/sources.list
echo "deb http://deb.debian.org/debian stretch-backports-sloppy main" >> /etc/apt/sources.list
apt-get update
apt-get install -y cmake/stretch-backports libuv1/stretch-backports libarchive13/stretch-backports-sloppy
