
/**
 *
@mainpage Davix Documentation
@author Devresse Adrien ( adrien.devresse@cern.ch )

From CERN IT-GT-DMS ( lcgutil-support@cern.ch )
mailing list : davix-devel@cern.ch


<h2> DAVIX </h2>

Davix is a lightweight toolkit for remote file interactions
      with HTTP based protocols.

Davix aims to supports all the needed features for an efficient file access and file management
in grid and clouds : SSL Session reuse,  X509 client auth, VOMS credential, S3 auth,
Vector operations (Partial reads, multi-range, single range), Partial PUT / PATCH, Fail-over, Multi-streams ( Metalinks), 
Redirection support for all operations, Redirections caching, Webdav parsing, Right Management (ACL), Meta-data functions ( mkdir, rmdir, unlink, etc.. ), Chunked transfert,
etc....

Davix supports the protocols
    - Http
    - WebDAV
    - Amazon S3

The Davix philosophy can be summarized as
    - Just Access Files, don't loose time with the protocol tricks
    - Keep It Simple Stupid
    - Lightweight, only the minimum set of dependencies
    - Portable


<h2> DAVIX API :</h2>

C++ API : \ref davix.hpp

<h2> Davix is yet an other libcurl ? </h2>

No,

- libcurl defines itself as a "client side URL transfer". <br/>
 it provides "protocol level" API, you compose your http queries mannually.

- Davix offers a "file level" API. <br/>
 With Davix, you access and manage your data and do not have to know anything about Http and
 how to tune queries.
 Davix tends to be of one level higher and provides a complete API for
 remote I/O and remote file management.

<h2> Examples : </h2>

<h3> POSIX API </h3>

 @code{.cpp}

            Davix::DavPosix p;
            p.stat("https://mywebdav-server.org/mydir/", &stat, &tmp_err);
            //
            // read ops
            fd= p.open(NULL, "https://mywebdav-server.org/myfile.jpg", O_RDONLY, &tmp_err);
            p.read(fd, buffer, size, &tmp_err);
            p.pread(fd, buffer, size2, offset, &tmp_err);
            p.close(fd);
            //
 @endcode

<h3> LOW LEVEL REQUEST API </h3>

 @code{.cpp}

            Davix::HttpRequest req("https://restapi-server.org/rest")
            req.addHeaderField(...)
            req.setRequestMethod("PUT")
            // .. configure ....
            //
            //
            // execute your request
            req.executeRequest(...);
 @endcode

<h2> What does Davix support ? </h2>

Davix supports :
    - all common posix operation ( stat/opendir/readdir/open/read/write/close/mkdir )
    - Webdav and XML parsing
    - remote random I/O
    - client side credential in PEM and P12 format
    - proxy certificate and voms extension
    - transparent redirection
    - third party copy with Webdav
    - keep alive and session re-use.
    - Basic authentication scheme

Davix is going to support soon :
    - transparent meta-link support with failover
    - vector operations
    - kerberos auth
    - transparent caching


<h2> How to compile : </h2>

- Davix Dependencies :
   -  openssl
   -  libxml-2.0
   -  Doxygen ( optional, for documentation generation )

- Davix Portability :
   - Should run on Windows and any POSIX compatible Operating system
   - Any contribution to support a new plateform is welcome
   - Tested on Debian, Ubuntu, Fedora 18/19, Scientific Linux 5/6
   - Tested on Windows under MinGW
   - Tested with clang and GCC.

- Compile :
    - " 1.svn export http://svn.cern.ch/guest/lcgdm/davix/trunk davix "
    - " 2.cd davix "
    - " 3.mkdir build; cd build"
    - " 4. cmake ../"
    - " 5. make "

- Generate doc :
    - * run cmake
    - make doc

- Compile and run unit tests :
    - cmake -DUNIT_TESTS=TRUE ../
    - make
    - make test

- Compile & execute func tests :
     * warning : functionals test needs davserver and a valid credential
    - " 4. cmake  -DFUNCTIONAL_TESTS=TRUE ../ "
    - " 5. . ../test/setup_test_env.sh
    - " 5.  make; make test"

- make RPMS :
    - ./packaging/bin/packager_rpm.sh ./packaging/rpm/specs/ ./
    - mock -r [mycfg] RPMS/davix-[...].src.rpm


<h2> Play with davix command line tool : </h2>
    davix has a set of command line tools for testing purpose and demonstration

        -> davix-ls: file listing
        -> davix-get: download operations
        -> davix-put: upload operations
        -> davix: low level query composition

<h2> TODO in Davix : </h2>

    - MacOSX portability check
    - Kerberos support
    - Metalink support

    please contact us on davix-devel@cern.ch ( CERN e-group & mailing list ) or  on adrien.devresse@cern.ch

    Any contribution is welcome

<h2> Davix Website : </h2>

        https://svnweb.cern.ch/trac/lcgutil/wiki

*/
