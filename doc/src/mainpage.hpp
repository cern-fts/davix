
/**
 *
@mainpage Davix Documentation
@author Devresse Adrien ( adrien.devresse@cern.ch )

Developped in CERN IT-SDC-ID 

Official WebSite: http://dmc.web.cern.ch/projects/davix/home 

User Documentation: http://dmc.web.cern.ch/projects/davix/documentation 

Mailing list : davix-devel@cern.ch


<h2> DAVIX </h2>

Davix is a lightweight toolkit for file access and file management with HTTP Based protocols.
Davix aims to be an easy-to-use, reliable and performant I/O layer for Cloud and Grid Storages.


Davix supports:
- SSL/TLS
- HTTP and SSL session reuse
- X509 client auth
- VOMS credential
- S3 auth,
- Vector operations (Partial reads, multi-range, single range)
- Partial PUT / PATCH *
- Fail-over
- Multi-streams ( Metalinks), *
- Redirection support for all operations
- Redirections caching
- Webdav parsing
- Right Management (ACL)
- Meta-data functions ( mkdir, rmdir, unlink, etc.. )
- Chunked transfert

 (*) still under development

Davix supports the protocols
    - Http
    - WebDAV
    - Aws S3

The Davix philosophy can be summarized as
    - Just access files, don't loose time with the protocol details
    - Keep It Simple Stupid
    - Efficient
    - Portable


<h2> DAVIX API :</h2>

File API : Davix::DavFile  <br/>

Posix-like API : Davix::DavPosix <br/>

Entry point API : \ref davix.hpp  <br/>


<h2> Davix is yet an other libcurl ? </h2>

In short : No

Libcurl defines itself as a "client side URL transfer". <br/>
 it provides "protocol level" API, you compose your http queries mannually.

- Davix offers a "file level" API. <br/>
 With Davix, you access and manage your data and do not have to know anything about Http and
 how to tune queries.
 Davix tends to be of one level higher and provides a complete API for
 remote I/O and remote file management.

<h2> Examples : </h2>

<h3> File Usage </h3>

Create a directory :
 @code{.cpp}
            DavixError* tmp_err=NULL;
            DavFile f(context, url);
            // creat directory
            p.makeCollection(NULL, &tmp_err);
 @endcode

 Get a full file content:
 @code{.cpp}

            DavixError* tmp_err=NULL;
            DavFile f(context, "http://mysite.org/file");
            int fd = open("/tmp/local_file", O_WRONLY | O_CREAT);
            // get full file
            if( p.getToFd(NULL,fd, &tmp_err) < 0)
                      std::cerr << "Error: " << tmp_err->getErrMsg() << std::endl;


 @endcode

Execute a partial GET :
 @code{.cpp}

            char buffer[255] = {0}
            DavixError* tmp_err=NULL;
            DavFile f(context, "http://mysite.org/file");
            // get 100 bytes from http://mysite.org/file after an offset of 200 bytes
            if( p.readPartial(NULL, buffer, 100, 200
                      &tmp_err) <0 )
                      std::cerr << "Error: " << tmp_err->getErrMsg() << std::endl;
            else
                std::cout << "Content: " << buffer << std::endl;

 @endcode

Execute a Vector Operation :
 @code{.cpp}

            char buffer[255] = {0}
            DavixError* tmp_err=NULL;
            DavFile f(context, "http://mysite.org/file");
            DavIOVecInput in[3];
            DavIOVecOutput ou[3];
            // get 100 bytes from http://mysite.org/file after an offset of 200 bytes
            if( p.readPartial(NULL, buffer, 100, 200
                      &tmp_err) <0 )
                      std::cerr << "Error: " << tmp_err->getErrMsg() << std::endl;
            else
                std::cout << "Content: " << buffer << std::endl;

 @endcode

<h3> POSIX Usage </h3>

 Stat query : 
 @code{.cpp}

            Davix::DavPosix p;
            // state quer
            p.stat("https://mywebdav-server.org/mydir/", &stat, &tmp_err);

 @endcode
 
 
 random I/O : 
 @code{.cpp}
             //
            // read ops
            fd= p.open(NULL, "https://mywebdav-server.org/myfile.jpg", O_RDONLY, &tmp_err);
            p.read(fd, buffer, size, &tmp_err);
            p.pread(fd, buffer, size2, offset, &tmp_err);
            p.close(fd);
            //
 @endcode


 
<h3> Query Usage </h3>

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
    - " 1. git clone  http://git.cern.ch/pub/davix  "
    - " 2. cd davix "
    - " 3. mkdir build; cd build"
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
    - map S3 bucket operations
    - ACL support
    - (?) CDMI support

    please contact us on davix-devel@cern.ch ( CERN e-group & mailing list ) or on adrien.devresse@cern.ch

    Any contribution is welcome

*/
