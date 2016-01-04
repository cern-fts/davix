Using the command line tools
============================

Here are some example invocations of the command line tools, along
with a description of what each one does.

davix-get
---------

davix-get allows you to download files using any of the supported protocols.

* Download a file and display the contents in standard output (typically terminal). ::

    $ davix-get http://example.org/dir/file_to_download

* Save the contents of a remote file locally on disk. ::

    $ davix-get http://example.org/dir/file_to_download local_file

* Download a collection (directory) of files. This is not possible to do with plain HTTP, as it doesn't support listing directories. ::

    $ davix-get -r20 dav://example.org/dir1/dir2 mydir

  davix-get will crawl through the entire directory tree beginning at dir2 (in this case with 20 threads) and attempt to download anything contained in dir2 and its sub-directories to mydir, if it has any. The original file hierarchy will be preserved.

* Download a collection of files from an S3 server. ::

    $ davix-get --s3accesskey xxxxx --s3secretkey yyyyy --s3region zzz s3://mybucket.example.org/collection mydir

davix-put
---------

davix-put allows you to upload local resources to a remote server using any of the supported protocols.

* Upload a local file. ::

    $ davix-put mydir/file_to_upload http://example.org/dir1/file

  In this case, davix will assume that the directory you're trying to upload to already exists.
  If you need to create one, have a look at davix-mkdir.

* Upload a local directory with all its contents. ::

    $ davix-put -r10 mydir http://example.org/dir1/dir2

  davix-put will crawl through the contents of a directory and its sub-directories and upload them to the remote server. (in this case using 10 threads).
  If the target directory does not already exist, it will attempt to create it. The original file hierarchy will be preserved.

* Upload a local directory to S3. ::

    $ davix-put --s3accesskey xxxxx --s3secretkey yyyyy --s3region zzz mydir s3://mybucket.example.org/collection

davix-ls
--------

davix-ls will list the contents of a remote directory for you. Note that it isn't possible to do a listing on a server
using plain HTTP, since the ``PROPFIND`` method from the WebDAV_ extensions is needed for that.

.. _WebDAV: https://en.wikipedia.org/wiki/WebDAV

* List the contents of a directory. ::

    $ davix-ls dav://example.org/dir1

* Do a recursive listing of the contents of a directory using N threads. ::

    $ davix-ls -rN dav://example.org/dir1

* List the contents of an S3 bucket. ::

    $ davix-ls --s3accesskey xxxxx --s3secretkey yyyyy --s3region zzz s3://mybucket.example.com

* By default, davix-ls treats an S3 bucket as a traditional, hierarchical file structure. It's also possible to do a flat listing,
  which includes all files in a bucket. ::

    $ davix-ls --s3accesskey xxxxx --s3secretkey yyyyy --s3-listing flat s3://mybucket.example.org

* Another possibility is a semi-hierarchical listing, which will list every file that starts with a prefix
  (eg dir1/dir2) but will also list every file in each subdirectory. ::

    $ davix-ls --s3accesskey xxxxx --s3secretkey yyyyy --s3-listing semi s3://mybucket.example.org/dir1/dir2

davix-mkdir
-----------

davix-mkdir will create an empty directory - plain HTTP servers are not supported.

* Create a collection on a WebDAV server. ::

    $ davix-mkdir dav://example.org/dir1

* Create a new bucket on an S3 server. ::

    $ davix-mkdir --s3accesskey xxxxx --s3secretkey yyyyy s3://mynewbucket.example.org

  Bucket names should not have any periods '.' in them, please refer to Amazon's bucket naming guidelines for more information.

davix-rm
--------

* Delete a file over plain HTTP. ::

    $ davix-rm http://example.org/file_to_delete

* Delete a file over S3. ::

    $ davix-rm --s3accesskey xxxxx --s3secretkey yyyyy s3://bucket.example.org/key_of_file_to_delete

* Recursively delete the contents of an S3 directory using N threads. ::

    $ davix-rm --s3accesskey xxxxx --s3secretkey yyyyy -rN s3://bucket.example.org/dir

  The command will remove all objects whose key starts with "dir/" using S3 multi-object delete. The number of object keys to include in each request
  can be adjusted using the -n switch. The S3 API supports up to 1000 keys per request, but in practice this may lead to connection timeouts. Default
  is set to 20.

* Delete a collection over WebDAV. ::

    $ davix-rm dav://example.org/collection/collection_to_delete

* Delete a bucket over S3. (has to be empty) ::

    $ davix-rm --s3accesskey xxxxx --s3secretkey yyyyy --s3region zzz s3://bucket_to_delete.example.org

davix-mv
--------

davix-mv allows you to rename or move resources over HTTP and WebDAV - there is no cloud support yet.

* Rename a resource. ::

    $ davix-mv http://example.org/rename_me http://example.org/thanks

* Move a resource to another location. ::

    $ davix-mv http://example.org/dir1/dir2/move_me http://example.org/move_me

davix-http
----------

You can use davix-http to construct and execute your own, hand-crafted HTTP requests. It can be used to interact with RESTful web-services.
If no request method is specified with the -X or --request option, davix-http will use GET by default.

* Execute an HTTP GET request and display the response to standard output. ::

    $ davix-http http://example.org

* Execute an HTTP PUT request to an S3 endpoint with simple payload. ::

    $ davix-http -X PUT s3://mybucket.example.org/myfile --data “Hello World” --s3accesskey xxxxx --s3secretkey yyyyy

  The above will create on the S3 server a file named myfile containing "Hello World".

* Execute an HTTP HEAD request over TLS with user authentication. ::

    $ davix-http -X HEAD https://login:password@example.org

* Execute a single-range GET request. ::

    $ davix-get https://example.org/file --header "Range: bytes=10-20"

  The above will print to standard output the file's specified range of bytes.

davix-cp
--------

davix-cp allows you to initiate HTTP third party copy on supporting services, providing you have the relevant credentials.

* Instruct the site *source* to transfer a file to site *destination*. ::

    $ davix-cp https://source/myfile https://destination/myfolder/myfile


