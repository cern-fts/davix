#!/usr/bin/env bash
set -e

yum install -y cmake git rpm-build yum-utils make which

if which dnf; then
  dnf install -y dnf-plugins-core
fi