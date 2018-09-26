#!/usr/bin/env bash

apt-get update
apt-get install -y git g++ cmake pbuilder devscripts equivs
mk-build-deps --install --remove "packaging/debian/control" --tool "apt-get -o Debug::pkgProblemResolver=yes --no-install-recommends -y"
