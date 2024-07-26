#!/usr/bin/env bash
set -e

git submodule update --init --recursive
./packaging/make-srpm.sh
cd build/
dnf builddep -y SRPMS/*

rpmbuild --rebuild --define "_build_name_fmt %%{NAME}-%%{VERSION}-%%{RELEASE}.%%{ARCH}.rpm" SRPMS/*
