# Davix

[![build status](https://gitlab.cern.ch/dmc/davix/badges/devel/build.svg)](https://gitlab.cern.ch/dmc/davix/commits/devel)

[Davix](http://dmc.web.cern.ch/projects/davix/home) aims to make the task of managing files over HTTP-based protocols simple. It is being developed by IT-ST at CERN, and while the project's purpose is its use on the [CERN grid](http://wlcg.web.cern.ch/), the functionality offered is generic.

## Documentation

Click [here](https://dmc-docs.web.cern.ch/dmc-docs/versions/docs/davix-epel/html/) to view the documentation of the latest released version, or [here](https://dmc-docs.web.cern.ch/dmc-docs/versions/docs/davix-development/html/) for the version under development. (devel branch)


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
   * cmake
   * libxml2-devel
   * openssl-devel

2. Compile:
```
git clone https://github.com/cern-fts/davix.git
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

## Development

The official repository is the one on [GitHub](https://github.com/cern-fts/davix). It's automatically mirrored on [CERN Gitlab](https://gitlab.cern.ch/dmc/davix) for CI purposes. This means:
* Use GitHub for new commits, issues, or pull requests. 
* Please don't commit directly on GitLab.
* After a commit, GitLab will mirror the changes automatically, and run CI. Treat Gitlab as if it were ie a Jenkins CI instance.

## Contact
Suggestions and patches are more than welcome. You can send an email to the [davix-devel](mailto:davix-devel@cern.ch) CERN mailing list, or contact directly the [current maintainer](mailto:georgios.bitzes@cern.ch).
