Using libdavix
==============

File interface
--------------

Most methods in the ``Davix::DavFile`` API come with an exception safe version
which accepts a ``DavixError*`` as an argument, and a version which throws ``DavixException``
instead.

* Instantiate a new DavFile.

.. code-block:: cpp

    Context c;
    DavFile file(c, Uri("http://example.org/dir1/file1"));

* Get all replicas associated to a file.

.. code-block:: cpp

    Context c;
    DavixError* err = NULL;
    DavFile file(c, Uri("http://example.org/dir1/file1"));
    std::vector<DavFile> resultVec;
    resultVec = file.getReplicas(NULL, &err);

* Execute a vector read operation.

.. code-block:: cpp

    Context c;
    DavixError* err = NULL;
    DavFile file(c, Uri("http://example.org/dir1/file_to_read"));

    int number_of_vector = 2;
    DavIOVecInput input_vector[number_of_vector];
    DavIOVecOutput output_vector[number_of_vector];

    // Setup vector operations parameters
    char buf1[255] = {0};
    char buf2[255] = {0};

    input_vector[0].diov_offset = 100;
    input_vector[0].diov_size = 200;
    input_vector[0].diov_buffer = buf1;

    input_vector[1].diov_offset = 600;
    input_vector[1].diov_size = 150;
    input_vector[1].diov_buffer = buf2;

    // execute query
    file.readPartialBufferVec(NULL, input_vector, output_vector, number_of_vector, &err);

    std::cout << "Op 1 read " << output_vector[0].diov_size << "bytes" << std::endl;
    std::cout << "Op 2 read " << output_vector[1].diov_size << "bytes" << std::endl;

    // do things with content in output_vector[0].diov_buffer etc

* Write the contents of a remote file to a local file descriptor.

.. code-block:: cpp

    Context c;
    DavixError* err = NULL;
    DavFile file(c, Uri("http://example.org/dir1/file_to_download"));
    int fd = open("/tmp/local_file" O_WRONLY, O_CREAT);
    // get full file
    file.getToFd(NULL, fd, &err);

    // get first 200 bytes from file
    file.getToFd(NULL, fd, 200, &err);

* Download parts of a file with a single-range GET

.. code-block:: cpp

    Context c;
    char buffer[255];
    DavFile file(c, Uri("http://example.org/dir1/file_to_download"));

    // get 100 bytes from http://example.org/dir1/file_to_download at offset 200
    file.readPartial(NULL, buffer, 100, 200);

* Download full file contents to a dynamically allocated buffer.

.. code-block:: cpp

    Context c;
    DavixError* err = NULL;
    DavFile file(c, Uri("http://example.org/file_to_download"));
    std::vector<char> buffer;

    // warning, this operation has no size limit regarding the content
    file.getFull(NULL, buffer, &err);

    // do things with buffer
    // ...

* Create or replace a remote file with the contents of a file descriptor.

.. code-block:: cpp

    Context c;
    DavFile file(c, Uri("http://example.org/file_to_create"));

    int fd = open("/tmp/file_to_upload", O_RDONLY);

    // get file size
    struct stat st;
    fstat(fd, &st);

    // execute put
    file.put(NULL, fd, static_cast<dav_size_t>(st.st_size));

* Create or replace a remote file with the contents of a buffer.

.. code-block:: cpp

    Context c;
    DavFile file(c, Uri("http://example.org/file_to_create"));

    char buffer[255];

    // fills buffer with something useful

    // execute put
    file.put(NULL, &buffer, static_cast<dav_size_t>sizeof(buffer));

* Create or replace a remote file with contents provided by a callback function.

.. code-block:: cpp

    // data provider
    int myDataProvider(void* buffer, dav_size_t max_size){
        static dav_size_t content_size = 200;
        if(max_size == 0)
            return 0;
        else{
            char my_useful_content[255]={1};
            int bytes_to_write = (max_size<content_size)?max_size:content_size;

            memcpy(buffer, my_useful_content, bytes_to_write);

            content_size -= bytes_to_write;
            return bytes_to_write;
        }
    }

    int main(int argc, char** argv){
        Context c;
        DavFile file(c, Uri("http://example.org/file_to_create"));

        // set data provider callback
        DataProviderFun dataCB = myDataProvider;

        // execute put and write 100 bytes using data from callback
        file.put(NULL, dataCB, 100);
    }

* Move a remote resource to another location.

.. code-block:: cpp

    Context c;
    DavFile source(c, Uri("http://example.org/old_location"));
    DavFile destination(c, Uri("http://example.org/new_location"));

    source.move(NULL, destination);

* Delete a collection or a directory.

.. code-block:: cpp

    Context c;
    DavixError* err = NULL;

    // delete a WebDAV collection
    DavFile myDavCollection(c, Uri("davs://example.org/collection_to_delete"));
    myDavCollection.deletion(NULL, &err);

    // to delete a S3 bucket (note: bucket has to be empty or operation will fail)
    // setup S3 authorisation keys
    RequestParams params;
    params.setAwsAuthorizationKeys("xxxxx", "yyyyy");
    DavFile myS3Bucket(c, Uri("s3://bucket_to_delete.example.org"));
    myS3Bucket.deletion(&params, &err);

* Create a collection or directory.

.. code-block:: cpp

    Context c;
    DavixError* err = NULL;
    // Instantiate RequestParams object to hold request options
    RequestParams params;

    // to create a WebDav collection
    DavFile myDavCollection(c, Uri("dav://example.org/collection_to_create"));
    myDavCollection.makeCollection(NULL, &err);

    // to create a new S3 bucket
    // first we need to setup S3 authorisation keys for this request
    params.setAwsAuthorizationKeys("xxxxx", "yyyyy");
    DavFile myS3Bucket(c, Uri("s3://bucket_to_create.example.org"));
    myS3Bucket.makeCollection(&params, &err);

* Query basic file metadata.

.. code-block:: cpp

    Contect c;
    DavFile file(c, Uri("http://example.org/dir/file_to_stat"));

    StatInfo info;
    file.stat(NULL, info);
    std::cout << "my file is " << info.size << " bytes large " << std::endl;
    std::cout << " mode : 0" << std::oct << info.mode << std::endl;
    std::cout << " atime : " << info.atime << std::endl;
    std::cout << " mtime : " << info.mtime << std::endl;
    std::cout << " ctime : " << info.ctime << std::endl;

* List the contents of a collection.

.. code-block:: cpp

    Contect c;
    DavFile file(c, Uri("http://example.org/collection_to_list"));

    DavFile::Iterator it = file.listCollection(NULL);

    // prints out entries' name
    do {
        std::cout << it.name() << std::endl;
    }while(it.next());

* Calculate a checksum

.. code-block:: cpp

    Context c;
    DavFile file(c, Uri("http://example.org/file_to_checksum"));
    std::string chk;

    // calculate MD5, also supports CRC32, ADLER32
    file.checksum(NULL, chk, "MD5", &err);
    std::cout << "MD5 " << chk << std::endl;

POSIX interface
---------------

* Instantiate a new DavPosix

.. code-block:: cpp

    Context c;
    DavPosix pos(&c);

* Query basic file metadata

.. code-block:: cpp

    Contect c;
    DavixError* err = NULL;
    DavPosix pos(&c);
    struct stat info;

    pos.stat(NULL, "http://example.org/file_to_stat", &info, &err);

    std::cout << " atime : " << info.st_atime << std::endl;
    std::cout << " mtime : " << info.st_mtime << std::endl;
    std::cout << " ctime : " << info.st_ctime << std::endl;
    std::cout << " mode : 0" << std::oct << info.st_mode << std::endl;
    std::cout << " len : " << info.st_size << std::endl;

* Open and read a collection

.. code-block:: cpp

    Context c;
    DavixError* err = NULL;
    DavPosix pos(&c);

    DAVIX_DIR* fd;
    struct dirent* entry;

    fd = pos.opendir(NULL, "dav://example.org/collection_to_open", &err);

    while(entry = pos.readdir(fd, &err)){
        std::cout << entry->d_name << std::endl;
    }

    pos.closedir(fd, &err);

* Open and read a collection with per-entry metadata

.. code-block:: cpp

    Context c;
    DavixError* err = NULL;
    DavPosix pos(&c);

    DAVIX_DIR* fd;
    struct dirent* entry;
    struct stat info;

    fd = posix.opendirpp(NULL, "dav://example.org/collection_to_open", &err);

    while(entry = pos.readdirpp(fd, &info, &err)){
        std::cout << entry->d_name << "is " << info.st_size << "bytes in size." << std::endl;
    }

    pos.closedirpp(fd, &err);

* Create a collection or directory

.. code-block:: cpp

    Context c;
    DavixError* err = NULL;
    DavPosix pos(&c);

    pos.mkdir(NULL, "dav://example.org/collection_to_create", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH, &err);

* Rename a file or collection

.. code-block:: cpp

    Context c;
    DavixError* err = NULL;
    DavPosix pos(&c);

    pos.rename(NULL, "http://example.org/myfolder/old_file_name", "http://example.org/myfolder/new_file_name", &err);

* Remove a file

.. code-block:: cpp

    Context c;
    DavixError* err = NULL;
    DavPosix pos(&c);

    pos.unlink(NULL, "http://example.org/file_to_delete", &err);

* Remove a collection or directory

.. code-block:: cpp

    Context c;
    DavixError* err = NULL;
    DavPosix pos(&c);

    pos.rmdir(NULL, "dav://example.org/collection_to_remove", &err);

* Open a file for random I/O (read, partial read, and write)

.. code-block:: cpp

    Context c;
    DavixError* err = NULL;
    DavPosix pos(&c);

    DAVIX_FD* fd;
    fd = pos.open(NULL, "http://example.org/myfile", O_RDONLY, &err);

    // read 200 bytes from myfile
    char buffer[255];
    pos.read(fd, &buffer, 200, &err);

    // read 50 bytes from myfile at offset 100
    char buffer2[255];
    pos.pread(fd, &buffer2, 50, 100, &err);
    pos.close(fd);

    // create a new file and write 200 bytes from buffer to it
    fd = pos.open(NULL, "http://example.org/myfolder/mynewfile", O_WRONLY | O_CREAT, &err);
    pos.write(fd, &buffer, 200);
    pos.close(fd);

* Vectored read - carry out several read operations in one single request, if the server supports it

.. code-block:: cpp

    Context c;
    DavixError* err = NULL;
    DavPosix pos(&c);

    int number_of_vector = 2;
    DavIOVecInput input_vector[number_of_vector];
    DavIOVecOuput output_vector[number_of_vector];

    DAVIX_FD* fd;
    fd = pos.open(NULL, "http://example.org/myfile", O_RDONLY, &err);

    // Setup vector operations parameters
    char buf1[255] = {0};
    char buf2[255] = {0};

    input_vector[0].diov_offset = 100;
    input_vector[0].diov_size = 200;
    input_vector[0].diov_buffer = buf1;

    input_vector[1].diov_offset = 600;
    input_vector[1].diov_size = 150;
    input_vector[1].diov_buffer = buf2;

    // execute query
    pos.preadVec(fd, input_vector, output_vector, number_of_vector, &err);

    std::cout << "Op 1 read " << output_vector[0].diov_size << "bytes" << std::endl;
    std::cout << "Op 2 read " << output_vector[1].diov_size << "bytes" << std::endl;

    // do things with content in output_vector[0].diov_buffer etc

    pos.close(fd);

* Re-position read/write file offset

.. code-block:: cpp

    Context c;
    DavixError* err = NULL;
    DavPosix pos(&c);

    DAVIX_FD* fd;
    fd = pos.open(NULL, "http://example.com/myfile", O_RDONLY, &err);

    // position cursor to 200 bytes offset
    lseek(fd, 200, SEEK_SET, &err);

    // position cursor to current location plus 100 offset
    lseek(fd, 100, SEEK_CUR, &err);

    // position cursor to end of the file plus offset 200
    lseek(fd, 200, SEEK_END, &err);

    pos.close(fd);

HTTP requests
-------------

The ``Davix::HttpRequest`` interface allows you to construct, customise and execute HTTP requests.
It also provides methods for retrieving server responses.

Requests can be executed in two ways:

* Using the ``executeRequest()`` method, which will execute the entire request immediately.
* Using the ``beginRequest()`` function, which will initiate a multi-part request. This should be used for requests that expect a large answer.
  Note that requests initiated by ``beginRequest()`` should be closed by using the ``endRequest()``.

* Instantiate a new HttpRequest

.. code-block:: cpp

    Context c;
    DavixError* err = NULL;
    HttpRequest myrequest(c, "http://example.org/some_useful_stuff", &err);

* Set the request method

.. code-block:: cpp

    myrequest.setRequestMethod("GET");

* Configure request parameters

.. code-block:: cpp

    RequestParams params;
    params.setUserAgent("MyAwesomeApp");
    params.setClientLoginPassword("my_login_name", "my_uber_secure_password");
    // ...
    myrequest.setParameters(params);

* Add custom header field

.. code-block:: cpp

    myrequest.addHeaderField("Accept", "application/metalink4+xml");

* Set the request body

.. code-block:: cpp

    // from a string
    std::string content_string
    myrequest.setRequestBody(content_string);

    // from a buffer
    char buffer [255];
    // fills buffer with something useful
    myrequest.setRequestBody(&buffer, sizeof(buffer));

    // from a file descriptor, at offset 100 for 200 bytes
    int fd = open("/tmp/myfile", O_RDONLY);
    myrequest.setRequestBody(fd, 100, 200);
    close(fd);

* Execute a full request

.. code-block:: cpp

    myrequest.executeRequest(&err);

* Start a multi-part HTTP request

.. code-block:: cpp

    myrequest.beginRequest(&err);

* End a multi-part HTTP request

.. code-block:: cpp

    myrequest.endRequest(&err);

* Read a block of size n bytes from the answer

.. code-block:: cpp

    // read max n bytes to static buffer
    char buffer[255];
    myrequest.readBlock(&buffer, n, &err);

    // read to dynamically sized buffer, with max size n
    std::vector<char> buffer2;
    myrequest.readBlock(&buffer2, n, &err);

* Read a segment of size n from the answer

.. code-block:: cpp

    // readSegment calls readBlock repeatedly until n size is read, or end of answer
    char buffer[50*1024];
    myrequest.readSegment(&buffer, n, &err);

* Read a line of text from the answer, with a maximum size of n

.. code-block:: cpp

    char buffer[255];
    myrequest.readLine(&buffer, n, &err);

* Write the answer contents to a file descriptor

.. code-block:: cpp

    char buffer[255]
    int fd = open("tmp/myfile", O_WRONLY | O_CREAT);

    // with no size limit
    myrequest.readToFd(fd, &err);

    // with 100 bytes limit
    myrequest.readToFd(fd, 100, &err);

* Get size of answer

.. code-block:: cpp

    dav_ssize_t size;
    size = myrequest.getAnswerSize();

* Get response body

.. code-block:: cpp

    // into dynamically sized buffer
    std::vector<char> buffer1;
    buffer1 = myrequest.getAnswerContentVec();

    // into static buffer
    const char* buffer2 = myrequest.getAnswerContent();

* Get the status code of the response

.. code-block:: cpp

    int code;
    code = myrequest.getRequestCode();

* Get last modified time

.. code-block:: cpp

    time_t last_modified;
    last_modified = myrequest.getLastModified();

* Get the value associated with a particular header key

.. code-block:: cpp

    std::string value;
    myrequest.getAnswerHeader("Content-Type", &value);
    std::cout << "Content-Type is " << value << std::endl;

* Get all header fields into a vector

.. code-block:: cpp

    HeaderVec headers;
    myrequest.getAnswerHeaders(headers);
    for(HeaderVec::iterator it = headers.begin(), it < headers.end(); ++it){
        std::cout << it->first << ": " << it->second << std::endl;
    }

* Clear the HttpRequest answer buffer

.. code-block:: cpp

    myrequest.clearAnswerContent();

* Process then discard the response body

.. code-block:: cpp

    // calls readSegment on the answer repeatedly but do nothing with content
    myrequest.discardBody(&err);

* Use pre-configured requests for specific HTTP operations

.. code-block:: cpp

    Context c;
    DavixError* err = NULL;
    Uri myuri("http://example.org/myfile");

    // Get request
    GetRequest req(c, myuri, &err);

    // Put request
    PutRequest req(c, myuri, &err);

    // Head request
    HeadRequest req(c, myuri, &err);

    // Delete request
    DeleteRequest req(c, myuri, &err);

    // Propfind request
    ProfindRequest req(c, myuri, &err);

Error reporting and exceptions
------------------------------

Davix uses an error reporting system similar to Glib, relying on ``Davix::DavixError`` objects to store the scope where an error occurred,
the error code, and the error message.

The ``DavixError`` objects can be checked on local level, or passed into other functions for error handling, or be propagated back up the stack frame.

Davix also provides a ``Davix::DavixException`` class that encapsulates ``DavixError``, for situations where exceptions are more appropriate.

Most functions in the ``Davix::DavFile`` API provide an exception safe version that takes a ``DavixError*`` as argument, as well as a version that throws ``DavixException`` and does not require ``DavixError``.

Davix::DavixError
~~~~~~~~~~~~~~~~~

The ``DavixError`` error reporting system can be used when exception throwing behavior is not desirable.

A ``DavixError`` object should not be instantiated manually, the ``DavixError::setupError()`` function should be used where an error has occurred.

Most functions provided by the APIs accept a pointer to the ``DavixError`` type, which if set to ``NULL``, will bypasses the error reporting system. In the event where an error occurs, a new ``DavixError`` object is created and its address assigned to the ``DavixError*`` passed into the function.

.. code-block:: cpp

    Context c;
    DavixError* err = NULL;

    // delete a WebDAV collection
    DavFile myDavCollection(c, Uri("davs://example.org/collection_to_delete"));
    myDavCollection.deletion(NULL, &err);

    // check if an error occurred
    // if err is not NULL anymore, that means a DavixError object had been created
    // if *err is not NULL, the object is valid
    if(err && *err){
        std::cerr << err->getErrScope() <<
                     " Error code: " << err->getStatus() <<
                     " Error: " << err->getErrMsg() << std::endl;
    }

Davix::DavixException
~~~~~~~~~~~~~~~~~~~~~

``DavixException`` is the recommended error handling method when using the ``DavFile`` API, it encapsulates a ``DavixError`` object which holds information such as the scope where an error occurred, the error code, and the error message.

* Throw a ``DavixException``

.. code-block:: cpp

    throw DavixException("Where", "What", "This is what has happened...");

* Catch a ``DavixException``

.. code-block:: cpp

    TRY_DAVIX{
        DavFile myDavCollection(c, Uri("davs://example.org/doomed_to_fail"));
            myDavCollection.deletion(NULL);
    }CATCH_DAVIX(&err){
        std::cerr << err->getErrScope() <<
        " Error code: " << err->getStatus() <<
        " Error: " << err->getErrMsg() << std::endl;

        // handle error or propagate
        // ...
    }
