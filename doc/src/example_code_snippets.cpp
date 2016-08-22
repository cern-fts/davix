/*
 * Example code snippets
*/

//-------------------------------------------------------------------------------------------------
// Davix::Uri
//-------------------------------------------------------------------------------------------------

//! [Uri example]
Uri myuri("https://johnsmith:12345678@example.org:443/myfolder/myfile?type=personal#overthere");
    cout << "Full uri: " << myuri.getString() << endl <<
        "Port: " << myuri.getPort() << endl <<
        "Protocol: " << myuri.getProtocol() << endl <<
        "Host: " << myuri.getHost() << endl <<
        "Path: " << myuri.getPath() << endl <<
        "Query: " << myuri.getQuery() << endl <<
        "Path and query: " << myuri.getPathAndQuery() << endl <<
        "User info: " << myuri.getUserInfo() << endl;

/*
Output:
Full uri: https://johnsmith:12345678@example.org:443/myfolder/myfile?type=personal#overthere
Port: 443
Protocol: https
Host: example.org
Path: /myfolder/myfile
Query: type=personal
Path and query: /myfolder/myfile?type=personal
User info: johnsmith:12345678
*/
//! [Uri example]


//-------------------------------------------------------------------------------------------------
// Davix::DavFile
//-------------------------------------------------------------------------------------------------

//! [DavFile]
Context c;
DavFile file(c, Uri("http://example.org/dir1/file1"));
//! [DavFile]

//! [getReplicas]
Context c;
DavixError* err = NULL;
DavFile file(c, Uri("http://example.org/dir1/file1"));
std::vector<DavFile> resultVec;
resultVec = file.getReplicas(NULL, &err);
//! [getReplicas]

//! [readPartialBufferVec]
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

std::cout << “Op 1 read ” << output_vector[0].diov_size << “bytes” << std::endl;
std::cout << “Op 2 read ” << output_vector[1].diov_size << “bytes” << std::endl;

// do things with content in output_vector[0].diov_buffer etc
//! [readPartialBufferVec]

//! [getToFd]
Context c;
DavixError* err = NULL;
DavFile file(c, Uri("http://example.org/dir1/file_to_download"));
int fd = open(“/tmp/local_file” O_WRONLY, O_CREAT);
// get full file
file.getToFd(NULL, fd, &err);
//! [getToFd]

//! [getToFd sized]
Context c;
DavixError* err = NULL;
DavFile file(c, Uri("http://example.org/dir1/file_to_download"));
int fd = open(“/tmp/local_file” O_WRONLY, O_CREAT);

// get 200 bytes from file
file.getToFd(NULL, fd, 200, &err);
//! [getToFd sized]

//! [readPartial]
Context c;
char buffer[255];
DavFile file(c, Uri(“http://example.org/dir1/file_to_download”));

// get 100 bytes from http://example.org/dir1/file_to_download at offset 200
file.readPartial(NULL, buffer, 100, 200);
//! [readPartial]

//! [getFull]
Context c;
DavixError* err = NULL;
DavFile file(c, Uri(“http://example.org/file_to_download”));
std::vector<char> buffer;

// warning, this operation has no size limit regarding the content
file.getFull(NULL, buffer, &err);

// do things with buffer
// ...
//! [getFull]

//! [get]
Context c;
DavixError* err = NULL;
DavFile file(c, Uri(“http://example.org/file_to_download”));
std::vector<char> buffer;

// warning, this operation has no size limit regarding the content
file.get(NULL, buffer);

// do things with buffer
// ...
//! [get]

//! [put fd]
Context c;
DavFile file(c, Uri(“http://example.org/file_to_create”));

int fd = open(“/tmp/file_to_upload”, O_RDONLY);

// get file size
struct stat st;
fstat(fd, &st);

// execute put
file.put(NULL, fd, static_cast<dav_size_t>(st.st_size));
//! [put fd]

//! [put buffer]
Context c;
DavFile file(c, Uri(“http://example.org/file_to_create”));

char buffer[255];

// fills buffer with something useful

// execute put
file.put(NULL, &buffer, static_cast<dav_size_t>sizeof(buffer));
//! [put buffer]

//! [put callback]
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
    DavFile file(c, Uri(“http://example.org/file_to_create”))'

    // set data provider callback
    DataProviderFun dataCB = myDataProvider;

    // execute put and write 100 bytes using data from callback
    file.put(NULL, dataCB, 100);
}
//! [put callback]

//! [move]
Context c;
DavFile source(c, Uri(“http://example.org/old_location”));
DavFile destination(c, Uri(“http://example.org/new_location”));

source.move(NULL, destination);
//! [move]

//! [delete]
Context c;

// to delete a WebDAV collection
DavFile myDavCollection(c, Uri(“davs://example.org/collection_to_delete”));
myDavCollection.deletion(NULL);

// to delete a S3 bucket (note: bucket has to be empty or operation will fail)
// setup S3 authorisation keys
RequestParams params;
params.setAwsAuthorizationKeys(“xxxxx”, “yyyyy”);
DavFile myS3Bucket(c, Uri(“s3://bucket_to_delete.example.org”));
myS3Bucket.deletion(&params);
//! [delete]

//! [delete no throw]
Context c;
DavixError* err = NULL;

// to delete a WebDAV collection
DavFile myDavCollection(c, Uri(“davs://example.org/collection_to_delete”));
myDavCollection.deletion(NULL, &err);

// to delete a S3 bucket (note: bucket has to be empty or operation will fail)
// setup S3 authorisation keys
RequestParams params;
params.setAwsAuthorizationKeys(“xxxxx”, “yyyyy”);
DavFile myS3Bucket(c, Uri(“s3://bucket_to_delete.example.org”));
myS3Bucket.deletion(&params, &err);
//! [delete no throw]


//! [makeCollection]
Context c;
DavixError* err = NULL;
// Instantiate RequestParams object to hold request options
RequestParams params;

// to create a WebDav collection
DavFile myDavCollection(c, Uri(“dav://example.org/collection_to_create”));
myDavCollection.makeCollection(NULL);

// to create a new S3 bucket
// first we need to setup S3 authorisation keys for this request
params.setAwsAuthorizationKeys(“xxxxx”, “yyyyy”);
DavFile myS3Bucket(c, Uri(“s3://bucket_to_create.example.org”));
myS3Bucket.makeCollection(&params);
//! [makeCollection]

//! [makeCollection no throw]
Context c;
DavixError* err = NULL;
// Instantiate RequestParams object to hold request options
RequestParams params;

// to create a WebDav collection
DavFile myDavCollection(c, Uri(“dav://example.org/collection_to_create”));
myDavCollection.makeCollection(NULL, &err);

// to create a new S3 bucket
// first we need to setup S3 authorisation keys for this request
params.setAwsAuthorizationKeys(“xxxxx”, “yyyyy”);
DavFile myS3Bucket(c, Uri(“s3://bucket_to_create.example.org”));
myS3Bucket.makeCollection(&params, &err);
//! [makeCollection no throw]

//! [statInfo]
Contect c;
DavFile file(c, Uri(“http://example.org/dir/file_to_stat”));

StatInfo info;
file.statInfo(NULL, info);
std::cout << "my file is " << info.size << " bytes large " << std::endl;
std::cout << " mode : 0" << std::oct << info.mode << std::endl;
std::cout << " atime : " << info.atime << std::endl;
std::cout << " mtime : " << info.mtime << std::endl;
std::cout << " ctime : " << info.ctime << std::endl;
//! [statInfo]


//! [stat]
Contect c;
DavixError* err = NULL;
DavFile file(c, Uri(“http://example.org/dir/file_to_stat”));

StatInfo info;
file.stat(NULL, info, &err);
std::cout << "my file is " << info.size << " bytes large " << std::endl;
std::cout << " mode : 0" << std::oct << info.mode << std::endl;
std::cout << " atime : " << info.atime << std::endl;
std::cout << " mtime : " << info.mtime << std::endl;
std::cout << " ctime : " << info.ctime << std::endl;
//! [stat]

//! [listCollection]
Contect c;
DavFile file(c, Uri(“http://example.org/collection_to_list”));

DavFile::Iterator it = file.listCollection(NULL);

// prints out entries' name
do{
	std::cout << it.name() << std::endl;
}while(it.next());
//! [listCollection]

//! [checksum]
Context c;
DavFile file(c, Uri(http://example.org/file_to_checksum));
std::string chk;

// calculate MD5, also supports CRC32, ADLER32
file.checksum(NULL, chk, “MD5”);
std::cout << “MD5 ” << chk << std::endl;
//! [checksum]

//! [checksum no throw]
Context c;
DavixError* err = NULL;
DavFile file(c, Uri(http://example.org/file_to_checksum));
std::string chk;

// calculate MD5, also supports CRC32, ADLER32
file.checksum(NULL, chk, “MD5”, &err);
std::cout << “MD5 ” << chk << std::endl;
//! [checksum no throw]


//-------------------------------------------------------------------------------------------------
// Davix::DavPosix
//-------------------------------------------------------------------------------------------------

//! [DavPosix]
Context c;
DavPosix pos(&c);
//! [DavPosix]

//! [stat]
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
//! [stat]

//! [stat64]
Contect c;
DavixError* err = NULL;
DavPosix pos(&c);
StatInfo info;

pos.stat64(NULL, "http://example.org/file_to_stat", &info, &err);

std::cout << "my file is " << info.size << " bytes large " << std::endl;
std::cout << " mode : 0" << std::oct << info.mode << std::endl;
std::cout << " atime : " << info.atime << std::endl;
std::cout << " mtime : " << info.mtime << std::endl;
std::cout << " ctime : " << info.ctime << std::endl;
//! [stat64]


//! [opendir]
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
//! [opendir]

//! [opendirpp]
Context c;
DavixError* err = NULL;
DavPosix pos(&c);

DAVIX_DIR* fd;
struct dirent* entry;
struct stat info;

fd = posix.opendirpp(NULL, "dav://example.org/collection_to_open", &err);

while(entry = pos.readdirpp(fd, &info, &err)){
	std::cout << entry->d_name << “is ” << info.st_size << “bytes in size.” << std::endl;
}

pos.closedirpp(fd, &err);
//! [opendirpp]

//! [mkdir]
Context c;
DavixError* err = NULL;
DavPosix pos(&c);

pos.mkdir(NULL, "dav://example.org/collection_to_create", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH, &err);
//! [mkdir]

//! [rename]
Context c;
DavixError* err = NULL;
DavPosix pos(&c);

pos.rename(NULL, "http://example.org/myfolder/old_file_name”, “http://example.org/myfolder/new_file_name", &err);
//! [rename]

//! [unlink]
Context c;
DavixError* err = NULL;
DavPosix pos(&c);

pos.unlink(NULL, "http://example.org/file_to_delete", &err);
//! [unlink]

//! [rmdir]
Context c;
DavixError* err = NULL;
DavPosix pos(&c);

pos.rmdir(NULL, "dav://example.org/collection_to_remove", &err);
//! [rmdir]

//! [open]
Context c;
DavixError* err = NULL;
DavPosix pos(&c);

DAVIX_FD* fd;
fd = pos.open(NULL, "http://example.org/myfile", O_RDONLY, &err);
//! [open]

//! [close]
Context c;
DavixError* err = NULL;
DavPosix pos(&c);

DAVIX_FD* fd;
fd = pos.open(NULL, "http://example.org/myfile", O_RDONLY, &err);
pos.close(fd);
//! [close]

//! [read]
Context c;
DavixError* err = NULL;
DavPosix pos(&c);

DAVIX_FD* fd;
fd = pos.open(NULL, "http://example.org/myfile", O_RDONLY, &err);

// read 200 bytes from myfile
char buffer[255];
pos.read(fd, &buffer, 200, &err);
pos.close(fd);
//! [read]

//! [pread]
Context c;
DavixError* err = NULL;
DavPosix pos(&c);

DAVIX_FD* fd;
fd = pos.open(NULL, "http://example.org/myfile", O_RDONLY, &err);

// read 50 bytes from myfile at offset 100
char buffer2[255];
pos.pread(fd, &buffer2, 50, 100, &err);
pos.close(fd);
//! [pread]

//! [write]
Context c;
DavixError* err = NULL;
DavPosix pos(&c);

DAVIX_FD* fd;
// create a new file and write 200 bytes from buffer to it
fd = pos.open(NULL, "http://example.org/myfolder/mynewfile", O_WRONLY | O_CREAT, &err);
pos.write(fd, &buffer, 200);
pos.close(fd);
//! [write]

//! [preadVec]
Context c;
DavixError* err = NULL;
DavPosix pos(&c);

int number_of_vector = 2;
DavIOVecInput input_vector[number_of_vector];
DavIOVecOutput output_vector[number_of_vector];

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
pos.pread_vec(fd, input_vector, output_vector, number_of_vector, &err);

std::cout << “Op 1 read ” << output_vector[0].diov_size << “bytes” << std::endl;
std::cout << “Op 2 read ” << output_vector[1].diov_size << “bytes” << std::endl;

// do things with content in output_vector[0].diov_buffer etc

pos.close(fd);
//! [preadVec]

//! [lseek]
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
//! [lseek]


//-------------------------------------------------------------------------------------------------
// Davix::HttpRequest
//-------------------------------------------------------------------------------------------------

//! [HttpRequest]
Context c;
DavixError* err = NULL;
HttpRequest myrequest(c, "http://example.org/some_useful_stuff", &err);
//! [HttpRequest]

//! [HttpRequest uri]
Context c;
DavixError* err = NULL;
HttpRequest myrequest(c, Uri("http://example.org/some_useful_stuff"), &err);
//! [HttpRequest uri]

//! [HttpRequest::setRequestMethod]
myrequest.setRequestMethod(“GET”);
//! [HttpRequest::setRequestMethod]

//! [HttpRequest::setsetParameters]
RequestParams params;
params.setUserAgent(“MyAwesomeApp”);
params.setClientLoginPassword(“my_login_name”, “my_uber_secure_password);
// ...
myrequest.setParameters(params);
//! [HttpRequest::setsetParameters]

//! [HttpRequest::addHeaderField]
myrequest.addHeaderField("Accept", "application/metalink4+xml");
//! [HttpRequest::addHeaderField]

//! [HttpRequest::setRequestBody]
// from a string
std::string content_string
myrequest.setRequestBody(content_string);

// from a buffer
char buffer [255];
// fills buffer with something useful
myrequest.setRequestBody(&buffer, sizeof(buffer));

// from a file descriptor, at offset 100 for 200 bytes
int fd = open(“/tmp/myfile”, O_RDONLY);
myrequest.setRequestBody(fd, 100, 200);
close(fd);
//! [HttpRequest::setRequestBody]

//! [HttpRequest::executeRequest]
myrequest.executeRequest(&err);
//! [HttpRequest::executeRequest]

//! [HttpRequest::beginRequest]
myrequest.beginRequest(&err);
//! [HttpRequest::beginRequest]

//! [HttpRequest::endRequest]
myrequest.endRequest(&err);
//! [HttpRequest::endRequest]

//! [HttpRequest::readBlock]
// read max n bytes to static buffer
char buffer[255];
myrequest.readBlock(&buffer, n, &err);

// read to dynamically sized buffer, with max size n
std::vector<char> buffer2;
myrequest.readBlock(&buffer2, n, &err);
//! [HttpRequest::readBlock]

//! [HttpRequest::readSegment]
// readSegment calls readBlock repeatedly until n size is read, or end of answer
char buffer[50*1024];
myrequest.readSegment(&buffer, n, &err);
//! [HttpRequest::readSegment]

//! [HttpRequest::readLine]
char buffer[255];
myrequest.readLine(&buffer, n, &err);
//! [HttpRequest::readLine]

//! [HttpRequest::readToFd]
char buffer[255]
int fd = open(“tmp/myfile”, O_WRONLY | O_CREAT);

// with no size limit
myrequest.readToFd(fd, &err);

// with 100 bytes limit
myrequest.readToFd(fd, 100, &err);
//! [HttpRequest::readToFd]

//! [HttpRequest::getAnswerSize]
dav_ssize_t size;
size = myrequest.getAnswerSize();
//! [HttpRequest::getAnswerSize]

//! [HttpRequest::getAnswerContentVec]
// into dynamically sized buffer
std::vector<char> buffer1;
buffer1 = myrequest.getAnswerContentVec();
//! [HttpRequest::getAnswerContentVec]

//! [HttpRequest::getAnswerContent]
// into static buffer
char buffer2[255];
buffer2 = myrequest.getAnswerContent();
//! [HttpRequest::getAnswerContent]

//! [HttpRequest::getRequestCode]
int code;
code = myrequest.getRequestCode();
//! [HttpRequest::getRequestCode]

//! [HttpRequest::getLastModified]
time_t last_modified;
last_modified = myrequest.getLastModified();
//! [HttpRequest::getLastModified]

//! [HttpRequest::getAnswerHeader]
std::string value;

myrequest.getAnswerHeader(“Content-Type”, &value);
std::cout << “Content-Type is ” << value << std::endl;
//! [HttpRequest::getAnswerHeader]

//! [HttpRequest::getAnswerHeaders]
HeaderVec headers;
myrequest.getAnswerHeaders(headers);
for(HeaderVec::iterator it = headers.begin(), it < headers.end(); ++it){
	std::cout << it->first << “: ” << it->second << std::endl;
}
//! [HttpRequest::getAnswerHeaders]

//! [HttpRequest::clearAnswerContent]
myrequest.clearAnswerContent();
//! [HttpRequest::clearAnswerContent]

//! [HttpRequest::discardBody]
myrequest.discardBody(&err);
//! [HttpRequest::discardBody]

//! [HttpRequest::GetRequest]
Context c;
DavixError* err = NULL;
Uri myuri(“http://example.org/myfile”);

// Get request
GetRequest req(c, myuri, &err);
//! [HttpRequest::GetRequest]

//! [HttpRequest::PutRequest]
Context c;
DavixError* err = NULL;
Uri myuri(“http://example.org/myfile”);

// Put request
PutRequest req(c, myuri, &err);
//! [HttpRequest::PutRequest]

//! [HttpRequest::HeadRequest]
Context c;
DavixError* err = NULL;
Uri myuri(“http://example.org/myfile”);

// Head request
HeadRequest req(c, myuri, &err);
//! [HttpRequest::HeadRequest]


//! [HttpRequest::DeleteRequest]
Context c;
DavixError* err = NULL;
Uri myuri(“http://example.org/myfile”);

// Delete request
DeleteRequest req(c, myuri, &err);
//! [HttpRequest::DeleteRequest]

//! [HttpRequest::ProfindRequest]
Context c;
DavixError* err = NULL;
Uri myuri(“http://example.org/myfile”);

// Propfind request
ProfindRequest req(c, myuri, &err);
//! [HttpRequest::ProfindRequest]

