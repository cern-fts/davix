
/**
 *
@mainpage Davix Documentation
@author CERN IT-GT-DMS ( lcgutil-support@cern.ch )
@author Devresse Adrien ( adrien.devresse@cern.ch )


<h2> DAVIX </h2>

Davix is a lightweight toolkit for remote file / object interactions
      with HTTP based protocols.

Davix offers a multi-layer API for the end user :

- High level POSIX file API, for convenience
- High level Object API, REST like
- Low level Request API, compose your own HTTP query like with curl. </br>

Davix supports Http, Webdav ( and S3 ) </br>
Davix integrate a Webdav / XML parser. </br>

<h2> DAVIX API :</h2>

C++ API : \ref davix.hpp

<h2> Davix is yet an other libcurl ? </h2>

No,

- libcurl defines itself as a "client side URL transfer". <br/>
 it provides "protocol level" API, you compose your http queries mannually.

- Davix offers a "file level" API. <br/>
 Davix tends to be of one level higher and
 provides a complete API for remote file operations on data and meta-data.

<h2> Examples : </h2>

<h3> POSIX API </h3>

 @code{.cpp}

            Davix::DavPosix p;
            p.stat("https://mywebdav-server.org/mydir/", &stat, &tmp_err);
            ####
            #### read ops
            fd= p.open(NULL, "https://mywebdav-server.org/myfile.jpg", O_RDONLY, &tmp_err);
            p.read(fd, buffer, size, &tmp_err);
            p.pread(fd, buffer, size2, offset, &tmp_err);
            ####
            #### directory creation
            p.mkdir(NULL, "https://mywebdav-server.org/mnewdir");
 @endcode

<h3> LOW LEVEL REQUEST API </h3>

 @code{.cpp}

            Davix::HttpRequest req("https://restapi-server.org/rest")
            req.addHeaderField(...)
            req.setRequestMethod("PUT")
            ###
            ### execute your request
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
   -  glib-2.0 ( will be removed )
   -  Doxygen ( optional, for doc generation )


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
    davix has a set of command line tool for Http/Webdav common operations

        -> davix ( low level and general purpose command line tool for HTTP Request, similar to curl )

<h2> I wish to see XYZ feature in Davix : </h2>

    Any suggestion is welcome.

    please contact us on davix-devel@cern.ch ( CERN e-group & mailing list ) or personnaly on adrien.devresse@cern.ch

    Davix is an open source and free project and will stay like this.
    Any contribution is welcome

<h2> Davix Website : </h2>

        https://svnweb.cern.ch/trac/lcgutil/wiki

*/
