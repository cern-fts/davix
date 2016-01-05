# Davix
[Davix](http://dmc.web.cern.ch/projects/davix/home) aims to make the task of managing files over HTTP-based protocols simple. It is being developed by the IT-SDC-ID section at CERN, and while the project's purpose is its use on the [CERN grid](http://wlcg.web.cern.ch/), the functionality offered is generic.

## HTTP File Management
HTTP is gaining popularity for file management tasks, beyond its traditional use for serving web pages. It is versatile enough to be fit for this purpose; `PUT`, `MOVE` and `DELETE` requests can be used for basic file manipulation, for example. (uploading, moving, and deleting a file, respectively)

Some common file-management operations are not possible to do with plain HTTP, however, which is why the [WebDAV](https://en.wikipedia.org/wiki/WebDAV) extensions were developed, which davix supports.

Davix also supports a plethora of authentication methods:
* x509 user certificate
* VOMS proxy
* RFC proxy with VOMS extensions support
* username / password
* AWS S3 compatible services
* Microsoft Azure compatible services

## Usage
Davix provides a shared library as well as a few command line tools. The library offers two sets of APIs, a file-oriented and a POSIX-like interface.

Here are some example invocations of the command-line tools.

```
# upload a file using a VOMS proxy
davix-put myfile https://someserver/dir/myfile -E /tmp/x509up_u1000
# download a file from an Amazon S3 bucket
davix-get https://mybucket.s3.amazonaws.com/somefile --s3accesskey [..] --s3secretkey [..]
# do an ls on a WebDAV-enabled server
davix-ls https://someserver/dir
```

## Compiling
1. Install the necessary dependencies:
   * boost, boost-devel
   * cmake
   * libxml2-devel
   * openssl-devel

2. Run the following commands. If you also want to compile the unit tests, add `-DUNIT_TESTS=TRUE` to the cmake invocation.
```
git clone https://github.com/cern-it-sdc-id/davix.git
cd davix
git submodule update --recursive --init
mkdir build && cd build
cmake ..
make
```

You can now try running an example command:
```
./src/tools/davix-get https://www.kernel.org/pub/linux/kernel/v4.x/ChangeLog-4.0.1
```

## Contact
Suggestions and patches are more than welcome. You can send an email to the [davix-devel](mailto:davix-devel@cern.ch) CERN mailing list, or contact directly the [current maintainer](mailto:georgios.bitzes@cern.ch).

## Contributors
* Adrien Devresse
* Fabrizio Furano
* Kwong Tat Cheung
* Georgios Bitzes
