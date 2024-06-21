#!/usr/bin/env bash
set -e

git submodule update --init --recursive
./packaging/make-srpm.sh
cd build

if which dnf; then
  dnf install -y epel-release || true
  dnf install -y dnf5-plugins || true # Fedora rawhide (FC41)
  dnf builddep -y SRPMS/*
else
  yum-builddep -y SRPMS/*
fi

rpmbuild --rebuild --define "_build_name_fmt %%{NAME}-%%{VERSION}-%%{RELEASE}.%%{ARCH}.rpm" SRPMS/*
