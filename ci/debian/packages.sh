#!/usr/bin/env bash
set -e

apt-get update
apt-get install -y git g++ cmake pbuilder devscripts equivs autoconf automake
mk-build-deps --install --remove "packaging/debian/control" --tool "apt-get -o Debug::pkgProblemResolver=yes --no-install-recommends -y"
