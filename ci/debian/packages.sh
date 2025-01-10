#!/usr/bin/env bash
set -e

apt update
apt install -y git g++ cmake pbuilder devscripts equivs autoconf automake
mk-build-deps --install --remove "packaging/debian/control" --tool "apt -o Debug::pkgProblemResolver=yes --no-install-recommends -y"
