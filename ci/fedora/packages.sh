#!/usr/bin/env bash
set -e

if [[ -f /usr/bin/dnf ]]; then
  dnf install -y dnf-plugins-core cmake git rpm-build make which
else
  yum install -y cmake git rpm-build yum-utils make which
fi
