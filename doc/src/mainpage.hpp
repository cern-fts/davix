
/**
 *
@mainpage Davix Documentation
@author Devresse Adrien ( adrien.devresse@cern.ch )

Developped at CERN (IT-SDC-ID)

Official WebSite: <a href="http://dmc.web.cern.ch/projects/davix/home">Here</a>

User Documentation: <a href="http://dmc.web.cern.ch/projects/davix/documentation">Here</a>

Mailing list : davix-devel@cern.ch

<h2> davix API :</h2>

file / object store API :   Davix::DavFile  <br/>
posix compatiblity layer :  Davix::DavPosix <br/>
http request layer:         Davix::HttpRequest <br/>
main header:                \ref davix.hpp  <br/>

<h2> What is davix ? </h2>

davix is a library and a set of tools for remote I/O on resources with HTTP based protocols.
It aims to be a simple, performant and portable I/O layer for Cloud and Grid Storages services.


Davix supports:
- HTTP, WebDav, Amazon S3
- SSL/TLS
- X509 client auth with proxy credential support
- Vector operations (Partial reads, multi-range, single range)
- Metalinks
- Redirections caching
- Webdav parsing
- Data management operations ( mkdir, rm, stat )

The Davix targets to:
- Be simple to use for simple use cases
- Provide the needed features for High Performance I/O use cases
- Be a data management swiss knife for HTTP based data stores

<h2> Examples </h2>


<h3>Query basic file metadata</h3>
 @code{.cpp}
            StatInfo infos;
            DavFile file(context, "http://my.webdav.server.org/myfolder/myfile");
            file.statInfo(NULL, infos);
            std::cout << "my file is " << infos.size << " bytes large " << std::endl;
 @endcode

<h3>Create a directory</h3>
 @code{.cpp}
            DavFile file(context, "http://my.webdav.server.org/myfoldier/newfolder");
            // creat directory
            file.makeCollection(NULL);
 @endcode

 <h3>Get a full file content</h3>
 @code{.cpp}

            DavFile f(context, "http://mysite.org/file");
            int fd = open("/tmp/local_file", O_WRONLY | O_CREAT);
            // get full file
            file.getToFd(NULL,fd, NULL) < 0)



 @endcode

<h3>Execute a partial GET</h3>
 @code{.cpp}

            char buffer[255] = {0};
            DavFile file(context, "http://mysite.org/file");
            // get 100 bytes from http://mysite.org/file after an offset of 200 bytes
            file.readPartial(NULL, buffer, 100, 200);

 @endcode

<h3>Execute a Vector Operation</h3>
 @code{.cpp}

            char buffer[255] = {0}
            DavFile file(context, "http://mysite.org/file");


            DavIOVecInput in[3];
            DavIOVecOutput ou[3];
            // setup vector operations parameters
            // --------
            // execute query
            file.readPartialBufferVec(NULL, in, out , 3 , NULL);


 @endcode

 <h3>Random I/O in posix mode</h3>
 @code{.cpp}

            Davix::DavPosix p;

            // read ops
            fd= p.open(NULL, "https://mywebdav-server.org/myfile.jpg", O_RDONLY, NULL);
            p.read(fd, buffer, size, NULL);
            p.pread(fd, buffer, size2, offset, NULL);
            p.close(fd);
            //
 @endcode



<h3>Manual HTTP query:</h3>
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


<h2> How to compile</h2>

- Davix Dependencies :
   -  openssl
   -  libxml-2.0
   -  Doxygen ( optional, for documentation generation )

- Davix Portability :
   - Target any POSIX OS
   - Ported on Linux > 2.6 , Windows with Cygwin, OSX > 10.2, AIX.
   - Packaged on Debian > 6, Ubuntu > 13.04, Fedora > 18, SL > 5,

- Compile :
    - " 1. git clone  http://git.cern.ch/pub/davix  "
    - " 2. cd davix "
    - " 3. mkdir build; cd build"
    - " 4. cmake ../"
    - " 5. make "

- Generate doc :
    - make doc

- Compile and run unit tests :
    - cmake ../
    - make
    - make test

- Compile & execute func tests :
     * warning : functionals test needs davserver and a valid credential
    - " 4. cmake  -DFUNCTIONAL_TESTS=TRUE ../ "
    - " 5. . ../test/setup_test_env.sh
    - " 5.  make; make test"

- make RPMS :
    - ./packaging/make-srpm.sh


<h2> Play with davix command line tool : </h2>
    davix has a set of command line tools for testing purpose and demonstration

        -> davix-ls: file listing
        -> davix-get: download operations
        -> davix-put: upload operations
        -> davix-http: low level query composition

<h2>TODO in Davix</h2>

    - WebHDFS support
    - S3 ACL support
    - CDMI support
    - X-stream mode support ( metalink multi source download )

    For any contribution please contact us on davix-devel@cern.ch


*/
