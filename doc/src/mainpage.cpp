
/**
    \mainpage Davix Documentation
    \author CERN IT-GT-DMS ( lcgutil-support@cern.ch )
    \author Devresse Adrien ( adrien.devresse@cern.ch )



    WARNING : Davix is still in a beta stage and the API can be subject to  modifications


    <h2> What is Davix ? </h2>

        Davix is a simple, lightweight and performant HTTP/Webdav library </br>

        Davix provides an POSIX-like API for file access,
        file management, and file transfer with Webdav/HTTP.

        Two interfaces are avaiables C++ and pure C.

        Davix supports :
            - all common posix operation ( stat/opendir/readdir/open/read/write/close )
            - remote random I/O
            - client side credential
            - proxy certificate
            - transparent redirection
            - third party copy with Webdav
            - session re-use.

        Davix is going to support soon :
            - transparent meta-link support with failover
            - vector operations
            - kerberos auth
            - transparent caching

        Davix is based on ab embedded and improved version of libneon

    <h2>API : </h2>

    - C++ API :
        \ref davix.hpp

    - Pure C API :
        \ref davix.h

    - Davix low level request API :
        \ref httprequest.hpp

    - Davix Error report system :
        \ref DavixError



    <h2>How to compile locally DAVIX</h2>
    - Compile :
        - " 1.svn export http://svn.cern.ch/guest/lcgdm/davix/trunk davix "
        - " 2.cd davix "
        - " 3.mkdir build; cd build"
        - " 4. cmake ../"
        - " 5. make "

    - Compile tests :
         * warning : functionals test needs davserver and a valid credential
        - " 4. cmake -DUNIT_TESTS=TRUE -DFUNCTIONAL_TESTS=TRUE ../ "
        - " 5. . ../test/setup_test_env.sh
        - " 5.  make; make test"

    - make RPMS :
        - ./packaging/bin/packager_rpm.sh ./packaging/rpm/specs/ ./
        - mock -r [mycfg] RPMS/davix-[...].src.rpm

    <h2>How to participate to DAVIX</h2>

        Davix is an open project and any contribution is welcome

        please contact us on davix-devel@cern.ch

*/
