Welcome to davix documentation
==============================

Davix aims to make the task of managing files over HTTP-based protocols simple. It is being developed by the IT-SDC-ID section at CERN, and while the project's purpose is its use on the CERN grid, the functionality offered is generic.

There is support for the following authentication methods:

* x509 user certificate
* VOMS proxy
* RFC proxy with VOMS extensions support
* username / password
* AWS S3 compatible services
* Microsoft Azure compatible services


.. toctree::
   :caption: Getting started
   :maxdepth: 2

   installation
   compiling

.. toctree::
   :caption: Guides
   :maxdepth: 2

   root
   cloud-support
   advanced

.. toctree::
   :caption: Examples
   :maxdepth: 2

   cli-examples
   lib-examples

.. toctree::
   :caption: API Reference
   :maxdepth: 2

   File-based API <doxygen/classDavix_1_1DavFile.html#://>
   POSIX-based API <doxygen/classDavix_1_1DavPosix.html#://>
   HTTP-based API <doxygen/classDavix_1_1HttpRequest.html#://>

.. toctree::
   :caption: About us
   :maxdepth: 2

   contact
   authors

.. toctree::
   :caption: Development
   :maxdepth: 2


Indices and tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`
