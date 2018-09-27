.. _compiling:

Compiling
=========

Install build dependencies
--------------------------

SL6/CentOS7
~~~~~~~~~~~

::

   sudo yum install openssl-devel libxml2-devel gsoap-devel \
                    doxygen cmake abi-compliance-checker

Ubuntu
~~~~~~

::

   sudo apt-get install abi-compliance-checker cmake debhelper doxygen \
                        gsoap libgridsite-dev libssl-dev libxml2-dev pkg-config

How to build
------------

Here is how to do a simple build of davix - have a look at the next section if you need to tweak some configuration option in cmake. ::

  git clone https://github.com/cern-fts/davix.git
  cd davix
  git submodule update --recursive --init
  mkdir build && cd build
  cmake ..
  make

Build options
-------------

Unit tests
~~~~~~~~~~

You can run the tests with ``make test``.

Functional tests
~~~~~~~~~~~~~~~~

Running functional tests requires authentication credentials, so they are not enabled by default. As a first step,
add ``-DFUNCTIONAL_TESTS=TRUE`` to cmake.

You will see that davix no longer compiles - it expects to find the file ``credentials/creds.cmake``. This
is the file which orchestrates which functional tests are run.
Here is an example - this is the file which runs our nightly build functional tests.
Passwords were removed for obvious reasons. ::

  ### tests using a proxy
  test_with_proxy("davs://dpmhead-rc.cern.ch/dpm/cern.ch/home/dteam/davix-tests")
  test_with_proxy("davs://prometheus.desy.de/VOs/dteam/davix-tests")

  ### AWS S3
  set(accesskey xxx)
  set(secretkey xxx)
  set(url https://some-bucket.s3.amazonaws.com/davix-tests)
  set(alt https://s3-ap-southeast-2.amazonaws.com/some-bucket/davix-tests)
  set(region ap-southeast-2)

  # test v2
  test_s3(${accesskey} ${secretkey} ${url} "" noalt)
  test_s3(${accesskey} ${secretkey} ${alt} "" alt)

  # test v4
  test_s3(${accesskey} ${secretkey} ${url} ${region} noalt)
  test_s3(${accesskey} ${secretkey} ${alt} ${region} alt)

  ### CERN ceph
  set(accesskey xxx)
  set(secretkey xxx)
  set(url s3s://some-bucket.cs3.cern.ch/davix-tests)

  test_s3(${accesskey} ${secretkey} ${url} "" noalt)

  ### Azure
  set(azurekey xxx)
  set(url https://some-user.blob.core.windows.net/some-bucket/davix-tests)

  test_azure(${azurekey} ${url})

Since this file contains sensitive information, access to it should be restricted and it should *never*
be committed to the source repository.

The ``test_with_proxy`` function uses the default grid-style proxy, ``/tmp/x509_u$uid``. It should be
generated beforehand.

To run the tests automatically, use the script under ``test/run-tests.sh``. This script further
expects the existence of ``credentials/obtain-proxy.sh``, which is run to generate a proxy
without any user intervention. Here is an example: ::

  #!/usr/bin/env bash
  echo "certificate_password_goes_here" | voms-proxy-init --cert $PWD/credentials/cert.p12 -pwstdin --voms dteam

Using ``test/run-tests.sh``, you can run automatic functional tests in jenkins, for example.


Generating the documentation
----------------------------

We use doxygen for the API documentation and sphinx for this how-to guide. Run ``make doc`` to generate both.
