.. _compiling:

Compiling
=========

Install build dependencies
--------------------------

SL6/CentOS7
~~~~~~~~~~~

::

   sudo yum install openssl-devel libxml2-devel gtest-devel gsoap-devel \
                    doxygen cmake boost-devel abi-compliance-checker

Ubuntu
~~~~~~

::

   sudo apt-get install abi-compliance-checker cmake debhelper doxygen \
                        gsoap libboost-system-dev libboost-thread-dev \
                        libgridsite-dev libgtest-dev libssl-dev libxml2-dev pkg-config \
                        rapidjson-dev

How to build
------------

Here is how to do a simple build of davix - have a look at the next section if you need to tweak some configuration option in cmake. ::

  git clone https://github.com/cern-it-sdc-id/davix.git
  cd davix
  git submodule update --recursive --init
  mkdir build && cd build
  cmake ..
  make

Build options
-------------

Unit tests
~~~~~~~~~~

Add ``-DUNIT_TESTS=TRUE`` to the cmake invocation, then run the tests with ``make test``.

Compile davix in embedded mode (no dependencies)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Add ``-D BOOST_EXTERNAL=NO`` to cmake.

Functional tests
~~~~~~~~~~~~~~~~

todo, write about credentials

Generating the documentation
----------------------------

Building packages
-----------------



