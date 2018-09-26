#!/usr/bin/env bash
set -e

yum install -y cmake git rpm-build yum-utils

if which dnf; then
  dnf install dnf-plugins-core
fi