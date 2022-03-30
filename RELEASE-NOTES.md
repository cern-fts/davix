# davix release history

## 0.8.1 (2022-03-22)
### Bug fixes
* [DMC-1259] - Gfal2-python pread(offset, count): Partial Content request not working against EOS storage
* [DMC-1279] - Davix with libcurl backend does not load certificate via callback functions
* [DMC-1291] - Davix fails to parse IPv6-format hostname during certificate verification
* [davix/issues/92] - Update CMake FindPackage Python to allow Python3 on macOS

### Changes
* [davix/pull/83] - Remove reva-specific credentials management

### Improvements
* [DMC-1025] - Allow HEAD-free open through posix API
* [DMC-1308] - Move Data Management Clients to Fedora 34 & 35
* [DMC-1313] - Provide Debian Stretch build for Davix
* [davix/issues/91] - Document the project Git branching model
* Many improvements fixing compiler warnings and building for Fedora platforms (Thanks to Mattias Ellert)

### Requests
* [DMC-1292] - Binary tarballs for davix

## 0.8.0 (2021-09-17)
### Epic
* [DMC-1267] - Davix with libcurl backend
* Huge refactoring to accommodate both libneon and libcurl backends, as well as improved testing

Many thanks to Georgios Bitzes for the great work. 

### Bug fixes
* [DMC-1209] - Davix Redirection Cache causes Segfault when encountering relative path
* [DMC-1243] - Davix hangs if url starts with 'https' is used with S3/Swift credentials in command
* [davix/pull/70] - Fix return value of HttpIO::readFull (Thanks to Max Orok)
* [davix/pull/66] - Initialize session factory members (Thanks to Petr Vokac)
* [davix/pull/54] - Really add '\0' after printed X.509 data (Thanks to Petr Vokac)

### New Features
* [DMC-1221] - Introduce a filter to avoid exposing sensitive information (such as bearer tokens) in Davix debug output
* [DMC-1238] - Add SWIFT support in Davix
* [DMC-1268] - Add CS3API support in Davix

Many thanks to Shiting Long for the Swift support.  
Many thanks to Rahul Chauhan for the CS3API support.

### Tasks
* [DMC-1250] - Packages for Centos8
* [DMC-1264] - Packages for Fedora >= 33

### Improvements
* [DMC-1245] - Add leading 0 to Adler32 checksum format in Davix

## 0.7.6 (2020-04-29)
### Bug fixes
* Ensure multi-range simulation thread exceptions are propagated
* Fix memory leak in S3 detect region function
* Recognize OpenSSL 'bad decrypt' error message as CredDecryptionError
* Fix error handling for proxy delegation

### Improvements
* Add protection in case server sends an unreasonable number of stripes during TPC
* Refactoring and improvements to davix-tester

Many thanks to Petr Vokac for fixing error handling during proxy delegation.

## 0.7.5 (2019-08-28)
### Bug fixes
* Enable use of dav:// and davs:// in third party copies.
* Fix third party copies when the server presents multiple certificate delegation endpoints. (Thanks to Thomas Hartmann for reporting)
* Prevent davix from infinite-looping when the server abruptly terminates the connection during TPC. (Thanks to Frank Berghaus for reporting)

### Improvements
* Add ability to cancel ongoing TPC transfers through user-supplied callback.
* Continued refactoring to eventually allow the use of libcurl as HTTP backend in davix.
* In-source builds are no longer supported, and explicitly prevented by CMake.

## 0.7.4 (2019-07-04)
### Bug fixes
* Correctly handle URL-encoded paths in PROPFIND responses (Thanks to Matthew Skinner for reporting)
* Prevent genversion.py from getting confused with non-davix git repositories (Thanks to Chris Burr)

### Improvements
* Fix cryptic cmake errors encountered sometimes when building from a tarball.

## 0.7.3 (2019-05-08)
### Bug fixes
* Use poll instead of select during async SSL connect - fixes a crash in certain cases of overload. (Thanks to Petr Vokac)

### Improvements
* Addition of davix docker image (Thanks to Emmanuel Frecon)
* Minor improvements to some error messages

## 0.7.2 (2019-02-15)
### Bug fixes
* produce-artifacts script was producing wonky release artifacts
* [DMC-1127] - davix should not segfault when calling DavPosix::close twice
* [DMC-1135] - davix misuses data provider function in S3 multi-part upload
* [DMC-1138] - Error from performance markers sometimes is not correctly reported by Davix
* [DMC-1140] - Fix parsing of the Digest to be complaint to RFC 3230

### Improvements
* Some refactoring and splitting of redirection caching logic into its own separate class.


## 0.7.1 (2018-10-24)
### Bug fixes
* [DMC-1114] - DAVIX adds cert chain multiple times
* Fixes to cmake related to linking of libuuid

## 0.7.0 (2018-10-22)

### Bug fixes
* [DMC-1089] - Implement S3 heuristic in davix to avoid STAT on pre-signed URLs
* [DMC-1090] - Add support for multi-part S3 uploads in davix
* [DMC-1094] - davix-put consumes too much memory during S3 and Azure multi-part uploads
* [DMC-1096] - Error when parsing iso8601 dates on FreeBSD
* [DMC-1097] - Davix AWS signature v2 and URL query
* [DMC-1101] - NEONRequest::readSegment always stops at line boundaries

### Improvements
* Improvements to release script and release procedure
* [DMC-1065] - Davix should read (and log) the response body even if it's an error
* [DMC-1098] - Implement the workflow for multipart S3 uploads through presigned URLs (dynafed)

## 0.6.9 (2018-09-25)

### Bug fixes
* [DMC-1063] - Incorrect handling of "Digest" header in response
* [DMC-1071] - Davix bug for push-mode third party COPY

### Improvements
* [DMC-1079] - force http/s as protocol scheme for COPY verb

## 0.6.8

### Bug fixes
* [DMC-1005] - davix buffers TPC performance markers in a weird way
* [DMC-1047] - Gridsite Delegation to dCache is broken ( starting from v 3.0)

### Improvements
* [DMC-1042] - Add gcloud support to davix
* [DMC-1013] - Add support for 3rd party copy transfers to S3 gCloud

## 0.6.7

### Bug fixes

* [DMC-950] - davix calculates invalid v4 s3 signature when using a non-default port
* [DMC-957] - davix appends an extra space with --header
* [DMC-961] - A neon session never retries after DNS lookup failure
* [DMC-969] - davix increments the refcount for OpenSSL structures in a non-atomic way
* [DMC-993] - Use-after-free in the davix cache

### Improvements

* [DMC-958] - Resume on xfer with metalinks creates file with a wrong filesize

## 0.6.6

### Bug fixes

* [DMC-937] - davix unable to upload files to Azure larger than a certain size
* [DMC-939] - Use SHA512 for 3rd party copy delegated proxy
* [DMC-944] - Remove the buggy davix fork-handler

### Improvements
* [DMC-938] - Remove boost from davix

## 0.6.5

### Bug fixes

* [DMC-877] - Davix seems to ignore the port number in a S3 URL
* [DMC-889] - davix should clear any old cached sessions after a fork
* [DMC-903] - davix does not always respect the option to disable transparent redirects
* [DMC-904] - davix does not properly escape URLs in recursive / parallel operations
* [DMC-912] - Davix seems not to honour the timeouts

### Improvements

* [DMC-867] - Honour 202-accepted and retry a request
* [DMC-876] - Setting log level and log scope should be thread-safe
* [DMC-887] - Switch over to using header parameters for the S3 signature
* [DMC-888] - Add support for openssl-1.1.0

## 0.6.4

### Bug fixes
* [DMC-824] - Impossible to upload directory if authentication method requires entering credentials manually
* [DMC-835] - Segfault in libneon when downloading large file with a bad network connection

### New features
* [DMC-871] - Add option to suppress 100-Continue when uploading files
* [DMC-874] - Need to report quota free space as well
* [DMC-816] - davix: Support for .netrc and .davixrc to store user credentials
* [DMC-850] - davix does not report used space in directories
* [DMC-854] - Expose davix version through its public headers

### Improvements
* [DMC-826] - Fix the clock_gettime / -lrt mess
* [DMC-828] - davix: Improve error message when receiving 507 Insufficient Storage

## 0.6.3

### Bug fixes
* [DMC-823] - The davix ROOT plugin does not build because of librt

## 0.6.2

### Bug fixes
* [DMC-821] - Code for multi-range requests allocates potentially huge buffer on the stack

## 0.6.1

### Bug fixes
* [DMC-815] - davix: Stray "\n" at the beginning of output when manually specifying a proxy
* [DMC-817] - gfal-copy output when copying to davs is quite odd

### Improvements
* [DMC-819] - davix: Script for 1-step release

## 0.6.0

### Bug fixes
* [DMC-194] - Davix has performance issues in case of fast small little chunk of data
* [DMC-207] - DAVIX : add option in order to report result of failing request
* [DMC-654] - Depending if the bucket is in the URL or not, the result is different
* [DMC-729] - Custom --header option in davix-get is sent twice on a redirect
* [DMC-732] - Davix: 120-second timeout with a large TCP buffer and a slow connection
* [DMC-735] - Davix trunk claims to be version 0.5.1, RPM says 0.5.0
* [DMC-736] - Davix parallel compilation has race condition
* [DMC-748] - Wrong filesize displayed when recursively listing directories
* [DMC-749] - davix-cp segfaults
* [DMC-752] - Cached redirects should expire after a DELETE
* [DMC-761] - Object-based interface returns an extra entry with empty filename in S3
* [DMC-762] - Race condition in libneon sometimes triggers erroneous timeout during SSL handshake
* [DMC-766] - Statting an S3 directory with a trailing slash returns wrong mode
* [DMC-770] - Fragment identifier not copied over when copying a Davix::Uri object
* [DMC-772] - TLS session persistence not working with Amazon S3 servers
* [DMC-773] - Davix crashes with an error when doing a vectored read on CEPH
* [DMC-778] - Davix static library crash on CentOS7
* [DMC-784] - Davix does not respect number of retries set by the user in case of 401 / 403
* [DMC-786] - Davix does not set DavixError nor throw an exception for uncommon server errors
* [DMC-789] - davix-mv does not filter out dav(s) protocol
* [DMC-799] - davix build on MacOS hangs if third party copy is enabled
* [DMC-805] - davix leaks file descriptors

### Improvements
* [DMC-739] - Support both S3 URL syntaxes
* [DMC-745] - Parse owner and group ids
* [DMC-760] - Create a robust functional test suite for davix
* [DMC-764] - Use a heuristic to find if the special Azure header needs to be added
* [DMC-765] - Make it possible to build davix using clang
* [DMC-780] - Add support for short-term S3 credentials
* [DMC-785] - Add explicit limit in maximum number of redirects, throw descriptive error if exceeded
* [DMC-683] - Davix 0.4.0 static library missing objects
* [DMC-728] - Add gtest as a git submodule in davix
* [DMC-753] - Make sure that a davix build fails if there are backwards-incompatible ABI chagnes
* [DMC-787] - Azure tests are flaky

### New Features
* [DMC-755] - Add Azure compatibility to Davix
* [DMC-774] - davix vectored reads - implement range coalescing
* [DMC-777] - Vectored reads using multiple connections and threads
* [DMC-611] - S3: Support for Signature Version 4 needed for new AWS regions
* [DMC-746] - Davix cannot create S3 directory
* [DMC-788] - Add support for S3 to davix-mv
* [DMC-791] - Create a consolidated and complete place for davix documentation
* [DMC-794] - Overwrite sensitive command line parameters

## 0.5.0

### Bug fixes
* [DMC-629] - Bulk davs deletion seems to fail every 18 operations as a clock
* [DMC-630] - Davix: davposix and davfile stat on S3 objects defaults to HTTP head request, bypasses signing
* [DMC-660] - Davix-put does not recognize the --verbose option ?
* [DMC-671] - Davix: S3 Uri transformer incorrectly assumes all operations are done without SSL
* [DMC-675] - Found a host that hangs Davix indefinitely
* [DMC-447] - Davix SSL Error needs to be clarified
* [DMC-569] - Davix: libneon has problems with the handling of corrupted or invalid ceritificate
* [DMC-691] - davix-put returns 0 ( =OK ) in case of permission denied
* [DMC-699] - Bad regression bugs
* [DMC-707] - Davix: xml parser does not handle multi-status response correctly for delete
* [DMC-193] - DAVIX: Transform and simply the rety mechanism on davix
* [DMC-633] - Connection timeout is sometimes triggered before the deadline when the global timeout is default.

### Improvements
* [DMC-614] - Davix: Add hierarchical file listing support for S3 bucket
* [DMC-621] - Davix: Add last modified date/time to S3 long listing
* [DMC-631] - Davix: Add virtual S3 directory stats request handling
* [DMC-634] - Davix: Add configurable max-keys param for S3 list object request
* [DMC-658] - Davix: Add functionality to support uploading and downloading collections
* [DMC-700] - Davix-get should not retry 10 times on err 403 (and maybe others)
* [DMC-702] - Davix: Implement Content-Type and Content-MD5 signing for S3 requests
* [DMC-715] - Davix: Implement multi-threaded crawler for recursive directory listing
* [DMC-716] - Davix: Allow user to specify the number of threads for recursive operations
* [DMC-718] - Davix can't copy a 0-sized file
* [DMC-624] - No decent error message if certificate or key are not readable
* [DMC-695] - Makes recursive operations on directory optional rather than default (davix-get/put)
* [DMC-703] - Davix: Remove gridsite dependency
* [DMC-708] - Davix: Add new parser for S3 multi-objects deletion response
* [DMC-711] - Davix: Update banner in source files with correct copyright info
* [DMC-713] - Shall the davix recursive crawler ignore more common errors and continue ?

### New Features
* [DMC-665] - Add support for 3rd party copy FROM dav TO s3
* [DMC-693] - Add support for pulling http 3rd party copies
* [DMC-668] - Davix-rm: bulk/folder remove

## 0.4.0

### Bug fixes
* [DMC-443] - Wrong section in .TH tag for libdavix.3
* [DMC-444] - Fix SSL Error related to too big digest (reported by Martin)
* [DMC-472] - Fix Davix follows redirection automatically even though redirectionSupport is switched off
* [DMC-501] - Fix SIGPIPE error with OpenSSL in Davix
* [DMC-518] - Fix POODLE vulnerability CVE-2014-3566 affects libneon, then davix
* [DMC-538] - Fix davix cmd line tool print an error in case of double authentication x509 -> login/password
* [DMC-547] - Update the functional tests
* [DMC-554] - Fix Thread-safety issue in the new davix logger system
* [DMC-564] - Fix issue: Libneon does not set OpenSSL thread callback for some plateforms ( Ubuntu )
* [DMC-571] - Fix compilation warnings on the libneon part
* [DMC-573] - Reinforce auto retry mechanism  (dCache Door problem)
* [DMC-581] - Solve portability problem under OSX/BSD due to the new hard timeout support
* [DMC-586] - Fix

### Improvements
* [DMC-474] - Log response body on http 3rd party copies
* [DMC-476] - Delegation v2 support needed
* [DMC-503] - Implement checksum support for S3 interface via Davix::File::checksum
* [DMC-534] - Davix: re-design the logger to be more user friendly and easier to tune

### New features
* [DMC-498] - S3 bucket creation support
* [DMC-499] - S3 checksum support
* [DMC-182] - Implement a list bucket feature for davix on the opendir/readdir model
* [DMC-505] - Hard timeout support
* [DMC-535] - Implement move operation in the File API
* [DMC-552] - DAVIX: improve PUT operation support in the file API

## 0.3.6

### Bug fixes
* [LCGUTIL-418] - DAVIX: Solve OpenSSL issue with davix cmd line tools and password shell
* [LCGUTIL-475] - davfile.getToFd does not return the number of bytes read, but 0 on success
* [LCGUTIL-478] - Davix 32 bits ABI break problem
* [LCGUTIL-480] - Davix maps badly http statuses to errno

## 0.3.4

### Bug fixes
* [LCGUTIL-410] - davix-cp help is not up to date and confusing
* [LCGUTIL-411] - davix-cp does not support the profile features of the davix command line tool
* [LCGUTIL-454] - Davix: libneon ignores connexion timeout
* [LCGUTIL-455] - DAVIX: bug reported by johannes concerning dCache and very long connexion re-use
* [LCGUTIL-456] - LCGUTIL: davix under heavy I/O usage for vector query send sometimes empty vector query

## 0.3.1

### New features
* Support for transparent fail-over based on Metalink, supported by all read I/O with HTTP
* Multi-Range support compatible with TDavixFile/ROOT 5/6
  * tested and working for dCache, DPM, Apache2, EOS, DynaFed, Owncloud, S3
* Implement SOCKS5 support based on libneon
* Add support for POSIX write operations, davix can now be used to write remotely on top of POSIX layer.
* Add getReplicas call, allowing to list replica of a resources using Metalink
* Introduce Checksum calculation feature
* Extend the command line tool with davix-mkdir, davix-rm
* Add long listing option to davix-ls -l
* Add the -P options to all command line tools, can be used to enable the usage of pre-defined profile
  * For instance "davix-ls -P grid davs://grid-storage.com/" enable all grid extensions for davix-ls
* Introduce a callback mechanism, allowing to intercept event inside davix based on std::function
* Drop internal library for boost ( dependency optional, git submodule )
* The "davix" http query tool is renamed "davix-http"
* First port on cygwin
* Add --headers options for OAuth spport to all command line tools
* Add --trace-headers options to all command line tools for query debugging
* create man pages for each cmd line tool
* Reduce default connextion timeout from 180s to 30s

### Improvements / bug fixes
* [LCGUTIL-170] - Davix : Correct important coverity warnings
* [LCGUTIL-275] - Double HEAD coming from gfal-copy/gfal2/davix (?) relies on KeepAlive
* [LCGUTIL-301] - Create Davix MetalinkParser and test it
* [LCGUTIL-302] - Implement replicas selector from Metalink parsers
* [LCGUTIL-103] - DAVIX: Evaluate performance of Http based IO and compare with other protocols ( gsiftp, xrootd )
* [LCGUTIL-154] - Davix: koji build hangs forever on rawhide
* [LCGUTIL-162] - DAVIX: portability issue seen on Win32 plateform
* [LCGUTIL-172] - Davix posix write does not handle failures correctly
* [LCGUTIL-174] - DAVIX : Add a request parameter flag in order to disable completely any session reuse
* [LCGUTIL-188] - Davix: prefixed install problem with cmake 2.8.7
* [LCGUTIL-196] - Davix & TDavixFile : map the main option to the ROOT Open file
* [LCGUTIL-318] - DAVIX: storm webdav parsing problem
* [LCGUTIL-341] - DAVIX: redirection caching problem with dCache instance
* [LCGUTIL-348] - gfal-copy produces misleading error
* [LCGUTIL-357] - gfal2-http fails to create the directories on 3rd party copies
* [LCGUTIL-383] - Merge fixes for 405 error mapping and discard body after a request is done
* [LCGUTIL-73] - DAVIX: design an API for advanced meta-link file management.
* [LCGUTIL-119] - DAVIX: implement a simple transparent failover case using meta-link.
* [LCGUTIL-344] - DAVIX : simplify davix code using boost, internal boost version is needed for ROOT usage
* [LCGUTIL-370] - DAVIX : rename davix cmd line tool to davix-http in order to avoid confusion
* [LCGUTIL-407] - DAVIX: Implement basic SOCKS5 support for Davix
* [LCGUTIL-408] - DAVIX: Add a "load module" feature to davix
* [LCGUTIL-113] - Davix misses a getReplicas function
* [LCGUTIL-115] - DAVIX: split clearly the C API and the C++ API
* [LCGUTIL-116] - DAVIX: export the IO buff map functions to the Object API
* [LCGUTIL-339] - DAVIX: add support for login/password HTTP auth when URL contains login/password
* [LCGUTIL-19] - add third party transfer support inside davix ( gfal 2.0 need )
* [LCGUTIL-175] - DAVIX: add options in order to display only request headers to the logging system
* [LCGUTIL-246] - DAVIX : Add support for davix command line tools for plain encrypted PEM credential
* [LCGUTIL-336] - DAVIX : error in case of separated cred and key usage
* [LCGUTIL-337] - Davix: Support 303 redirection code for GET Operation too
* [LCGUTIL-338] - Davix : Support for file request parameter level personalized header
* [LCGUTIL-369] - Davix: add cmd line tool for simple collection creation ( mkdir )
* [LCGUTIL-371] - DAVIX : abi break between 0.2.8, and 0.3 : an ABI break has been detected on davix, this need to be fixed
* [LCGUTIL-372] - DAVIX: add simple stupid option --headers to display request headers of any operations with davix
* [LCGUTIL-373] - DAVIX : fix compilation problems and warning on OSX with clang compilation
* [LCGUTIL-375] - DAVIX: prepare internal IO stack for Metalink usage, Rucio parsing and s3 bucket parsing
* [LCGUTIL-379] - DAVIX: Fix misleading Connexion timeout debug message
* [LCGUTIL-381] - DAVIX: include man page for the software distribution
* [LCGUTIL-394] - DAVIX: bad_alloc throw by davix in some case with dCache endpoints
* [LCGUTIL-409] - DAVIX: implement POSIX partial write support with davix

## 0.2.8

### Bug fixes
* [LCGUTIL-197] - Davix : bug inside the vector request system : vector request split to "one" range can trigger parsing problem

### Improvements
* [LCGUTIL-333] - Improve Davix setup on OSX
* [LCGUTIL-327] - DAVIX: add long listing support to davix-ls
* [LCGUTIL-274] - add an error code "redirection needed" to davix status errors

## 0.2.7

### Improvements
* remove several GNU ext dependency
* first version stable for TDavixFile/TDavixSystem
* include prefetching support
* several minor bug fixes
* fix a problem related to stat() mode flag in plain http mode
* clean of old C error code system

## 0.2.7

### Improvements
* support for S3 auth tokens
* add external gtest support
* simplify build system
* remove strlcpy dependency
* add several Meta-data operation support
* support for very large Vector IO query ( > 1000 chunks )
* support for IO prefecthing
* add initial tools
* bug fixe in the session reuse system
* bug fixe in the redirection system
* add stream support for Davix Uri
* clean and re-organize headers
* add support for dav:// davs:// s3:// and s3s:// schemes
* several warnings correction from coverity scan
* resolve several packaging issues
* Inclusion of the prototype davix_copy feature: third party copy based on HTTP

## 0.2.0

### Improvements
* Initial Stable version
* Support POSIX and FILE API
* Remote I/O read only
* support for S3
* support for X509 / VOMS / Proxy credential
* support for Vector IO
* support for session reuse
