#!/usr/bin/env bash
set -e

git submodule update --init --recursive
./packaging/make-srpm.sh
cd build

if which dnf; then
  dnf install -y epel-release || true
  dnf builddep -y SRPMS/*
else
  yum-builddep -y SRPMS/*
fi

rpmbuild --rebuild --with server --define "_build_name_fmt %%{NAME}-%%{VERSION}-%%{RELEASE}.%%{ARCH}.rpm" SRPMS/*
