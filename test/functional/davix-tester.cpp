#include <stdio.h>
#include <iostream>
#include <vector>
#include "optionparser.h"
#include <sstream>
#include <stdexcept>

#include <davix.hpp>
#include "davix_test_lib.h"
#include "utils/davix_s3_utils.hpp"

using namespace Davix;

#define SSTR(message) static_cast<std::ostringstream&>(std::ostringstream().flush() << message).str()
#define DECLARE_TEST() std::cout << " ----- Performing test: " << __FUNCTION__ << " on " << uri << std::endl

#include "lorem-ipsum.h" // define std::string teststring
const std::string testfile("davix-testfile-");

#define ASSERT(assertion, msg) \
    if((assertion) == false) throw std::runtime_error( SSTR(__FILE__ << ":" << __LINE__ << " (" << __func__ << "): Assertion " << #assertion << " failed.\n" << msg))

void initialization(int argc, char** argv) {
    std::cout << "Command: ";
    for(int i = 0; i < argc; i++) {
        std::cout << std::string(argv[i]) << " ";
    }
    std::cout << std::endl;
}

std::vector<std::string> split(const std::string str, const std::string delim) {
    size_t prev = 0, cur;
    std::vector<std::string> results;
    while((cur = str.find(delim, prev)) != std::string::npos) {
        results.push_back(str.substr(prev, cur-prev));
        prev = cur + delim.size();
    }
    std::string last = str.substr(prev, str.size()-prev);
    if(last.size() != 0)
        results.push_back(last);

    return results;
}

namespace Auth {
enum Type {AWS, PROXY, AZURE, NONE, ILLEGAL};
Type fromString(const std::string &str) {
    if(str == "aws")
        return Auth::AWS;
    if(str == "proxy")
        return Auth::PROXY;
    if(str == "azure")
        return Auth::AZURE;
    if(str == "none")
        return Auth::NONE;

    return Auth::ILLEGAL;
};
};

static option::ArgStatus option_nonempty(const option::Option& option, bool msg) {
    if (option.arg != 0 && option.arg[0] != 0)
        return option::ARG_OK;
    if (msg) std::cout << "Option '" << option << "' requires a non-empty argument" << std::endl;
        return option::ARG_ILLEGAL;
}


namespace Opt {
enum  Type { UNKNOWN, HELP, AUTH, S3ACCESSKEY, S3SECRETKEY, S3REGION,
                    AZUREKEY, S3ALTERNATE, CERT, URI, TRACE, COMMAND };
}

bool verify_options_sane(option::Parser &parse, std::vector<option::Option> &options) {
    if(parse.error()) {
        std::cout << "Parsing error" << std::endl;
        return false;
    }

    if(options[Opt::HELP]) {
        return false;
    }

    for(option::Option* opt = options[Opt::UNKNOWN]; opt; opt = opt->next()) {
        std::cout << "Unknown option: " << std::string(opt->name,opt->namelen) << "\n";
        return false;
    }

    for(int i = 0; i < parse.nonOptionsCount(); ++i) {
        std::cout << "Non-option #" << i << ": " << parse.nonOption(i) << "\n";
        return false;
    }

    return true;
}

std::vector<option::Option> parse_args(int argc, char** argv) {
    const option::Descriptor usage[] = {
        {Opt::UNKNOWN, 0, "", "", option::Arg::None, "davix functional tests runner\n"
                                                     "USAGE: tester [options]\n\n" "Options:" },
        {Opt::HELP, 0, "", "help", option::Arg::None, " --help \tPrint usage and exit." },
        {Opt::AUTH, 0, "", "auth", option_nonempty, " --auth \t Authentication method" },
        {Opt::S3ACCESSKEY, 0, "", "s3accesskey", option_nonempty, " --s3accesskey S3 access key"},
        {Opt::S3SECRETKEY, 0, "", "s3secretkey", option_nonempty, " --s3secretkey S3 secret key"},
        {Opt::S3REGION, 0, "", "s3region", option_nonempty, "--s3region S3 region"},
        {Opt::AZUREKEY, 0, "", "azurekey", option_nonempty, "--azurekey Azure key"},
        {Opt::S3ALTERNATE, 0, "", "s3alternate", option::Arg::None, "--s3alternate"},
        {Opt::CERT, 0, "", "cert", option_nonempty, "--cert path to the proxy certificate to use"},
        {Opt::URI, 0, "", "uri", option_nonempty, "--uri uri to test against"},
        {Opt::TRACE, 0, "", "trace", option_nonempty, "--trace debug scope"},
        {Opt::COMMAND, 0, "", "command", option_nonempty, "--command test to run"},
        {Opt::UNKNOWN, 0, "", "",option::Arg::None, "\nExamples:\n"
                                               "  tester --auth proxy --uri https://storage/davix-tests --command makeCollection" },

        {0,0,0,0,0,0}
    };

    option::Stats stats(usage, argc-1, argv+1); // TODO fix argc-1
    std::vector<option::Option> options(stats.options_max);
    std::vector<option::Option> buffer(stats.buffer_max);
    option::Parser parse(usage, argc-1, argv+1, &options[0], &buffer[0]);

    if(!verify_options_sane(parse, options)) {
        option::printUsage(std::cout, usage);
        exit(1);
    }

    return options;
}

std::string retrieve(const std::vector<option::Option> &options,  const Opt::Type key) {
    if(!options[key]) return "";
    return options[key].arg;
}

void authentication(const std::vector<option::Option> &opts, const Auth::Type &auth, RequestParams &params) {
    if(auth == Auth::AWS) {
        params.setProtocol(RequestProtocol::AwsS3);

        ASSERT(opts[Opt::S3ACCESSKEY] != NULL, "--s3accesskey is required when using s3");
        ASSERT(opts[Opt::S3SECRETKEY] != NULL, "--s3secretkey is required when using s3");

        params.setAwsAuthorizationKeys(retrieve(opts, Opt::S3SECRETKEY), retrieve(opts, Opt::S3ACCESSKEY));
        if(opts[Opt::S3REGION]) params.setAwsRegion(retrieve(opts, Opt::S3REGION));
        if(opts[Opt::S3ALTERNATE]) params.setAwsAlternate(true);
    }
    else if(auth == Auth::PROXY) {
        configure_grid_env("proxy", params);
    }
    else if(auth == Auth::AZURE) {
        ASSERT(opts[Opt::AZUREKEY] != NULL, "--azurekey is required when using Azure");

        params.setProtocol(RequestProtocol::Azure);
        params.setAzureKey(retrieve(opts, Opt::AZUREKEY));
    }
    else {
        ASSERT(false, "unknown authentication method");
    }
}

void depopulate(const RequestParams &params, Uri uri, int nfiles) {
    DECLARE_TEST();

    Context context;
    for(int i = 1; i <= nfiles; i++) {
        Uri u(uri);
        u.addPathSegment(SSTR(testfile << i));
        DavFile file(context, params, u);
        file.deletion(&params);
        std::cout << "File " << i << " deleted successfully." << std::endl;
    }
    std::cout << "All OK" << std::endl;
}

std::string string_from_mode(mode_t mode){
    const char* rmask ="xwr";
    std::string str(10,'-');

    str[0] = (S_ISDIR(mode))?'d':'-';
    for(size_t i=0; i < 9; ++i){
        str[9-i] = (( mode & (0x01 << i))?(rmask[i%3]):'-');
    }
    return str;
}

void statdir(const RequestParams &params, Uri uri) {
    DECLARE_TEST();
    Context context;
    DavFile file(context, params, uri);
    StatInfo info;
    file.statInfo(&params, info);
    std::cout << string_from_mode(info.mode) << std::endl;

    ASSERT(S_ISDIR(info.mode), "not a directory");
}

void makeCollection(const RequestParams &params, Uri uri) {
    DECLARE_TEST();

    Context context;
    DavFile file(context, params, uri);
    file.makeCollection(&params);

    // make sure it is empty
    DavFile::Iterator it = file.listCollection(&params);
    ASSERT(it.name() == "" && !it.next(), "Newly created directory not empty!");

    // do a stat, make sure it's a dir
    statdir(params, uri);

    Uri u2 = uri;
    u2.ensureTrailingSlash();
    statdir(params, u2);

    std::cout << "Done!" << std::endl;
}

#define NEON_S3_SIGN_DURATION 3600

void statfileFromSignedURI(const RequestParams &params, const Uri uri) {
  DECLARE_TEST();

  Uri signedURI(S3::signURI(params, "GET", uri, params.getHeaders(), NEON_S3_SIGN_DURATION));
  RequestParams params2(params);

  signedURI.httpizeProtocol();

  params2.setProtocol(RequestProtocol::Http);
  params2.setAwsAuthorizationKeys("", "");
  params2.setAwsRegion("");
  params2.setAwsToken("");

  Context context;
  DavFile file(context, params2, signedURI);
  StatInfo info;
  file.statInfo(&params2, info);
  std::cout << string_from_mode(info.mode) << std::endl;
  std::cout << info.size << std::endl;

  ASSERT(! S_ISDIR(info.mode), "not a file");
}

/* stat a file, make sure it's a file */
void statfile(const RequestParams &params, const Uri uri) {
    DECLARE_TEST();
    Context context;
    DavFile file(context, params, uri);
    StatInfo info;
    file.statInfo(&params, info);
    std::cout << string_from_mode(info.mode) << std::endl;
    std::cout << info.size << std::endl;

    ASSERT(! S_ISDIR(info.mode), "not a file");

    if(!params.getAwsAutorizationKeys().first.empty()) {
      // Now try statting through the signed URL
      statfileFromSignedURI(params, uri);
    }
}

void movefile(const RequestParams &params, const Uri uri) {
    DECLARE_TEST();
    Context context;
    Uri u1(uri);
    Uri u2(uri);

    u1.addPathSegment(SSTR(testfile << 1));
    u2.addPathSegment(SSTR(testfile << 1 << "-moved"));

    DavFile source(context, params, u1);
    DavFile dest(context, params, u2);

    source.move(&params, dest);
    statfile(params, u2);
    dest.move(&params, source);
}

void populate(const RequestParams &params, const Uri uri, const int nfiles) {
    DECLARE_TEST();

    Context context;
    for(int i = 1; i <= nfiles; i++) {
        Uri u(uri);
        u.addPathSegment(SSTR(testfile << i));
        DavFile file(context, params, u);
        file.put(NULL, testString.c_str(), testString.size());
        std::cout << "File " << i << " uploaded successfully." << std::endl;
        std::cout << u << std::endl;

        statfile(params, u);
    }
}

// count the number of files in folder
void countfiles(const RequestParams &params, const Uri uri, const int nfiles) {
    DECLARE_TEST();
    Context context;
    DavFile file(context, params, uri);
    DavFile::Iterator it = file.listCollection(&params);
    int i = 0;

    do {
        i++;
    } while(it.next());

    ASSERT(i == nfiles, "wrong number of files; expected " << nfiles << ", found " << i);
    std::cout << "All OK" << std::endl;
}

// confirm that the files listed are the exact same ones uploaded during a populate test
void listing(const RequestParams &params, const Uri uri, const int nfiles) {
    DECLARE_TEST();
    int hits[nfiles+1];
    for(int i = 0; i <= nfiles; i++) hits[i] = 0;

    Context context;
    DavFile file(context, params, uri);
    DavFile::Iterator it = file.listCollection(&params);

    int i = 0;
    do {
        i++;
        std::string name = it.name();
        std::cout << "Found " << name << std::endl;

        // make sure the filenames are the same as the ones we uploaded
        ASSERT(name.size() > testfile.size(), "Unexpected filename: " << name);
        std::string part1 = name.substr(0, testfile.size());
        std::string part2 = name.substr(testfile.size(), name.size()-1);

        ASSERT(part1 == testfile, "Unexpected filename: " << part1);
        int num = atoi(part2.c_str());
        ASSERT(num > 0, "Unexpected file number: " << num);
        ASSERT(num <= nfiles, "Unexpected file number: " << num);
        hits[num]++;
    } while(it.next());

    // count all hits to make sure all have exactly one
    ASSERT(i == nfiles, "wrong number of files; expected " << nfiles << ", found " << i);
    for(int i = 1; i <= nfiles; i++)
        ASSERT(hits[i] == 1, "hits check for file" << i << " failed. Expected 1, found " << hits[i]);

    std::cout << "All OK" << std::endl;
}

/* upload a file and move it around */
void putMoveDelete(const RequestParams &params, const Uri uri) {
    DECLARE_TEST();
    Uri u = uri;
    Uri u2 = uri;
    u.addPathSegment(SSTR(testfile << "put-move-delete"));
    u2.addPathSegment(SSTR(testfile << "put-move-delete-MOVED"));

    Context context;
    DavFile file(context, params, u);
    file.put(&params, testString.c_str(), testString.size());

    DavFile movedFile(context, params, u2);
    file.move(&params, movedFile);

    movedFile.deletion(&params);
    std::cout << "All OK" << std::endl;
}

void remove(const RequestParams &params, const Uri uri) {
    DECLARE_TEST();

    // a very dangerous test.. Make sure that uri at least
    // contains "davix-test" in its path.
    bool safePath = uri.getPath().find("davix-test") != std::string::npos;
    ASSERT(safePath, "Uri given does not contain the string 'davix-test'. Refusing to perform delete operation for safety.");

    Context context;
    DavFile file(context, params, uri);
    file.deletion(&params);
}

void preadvec(const RequestParams &params, const Uri uri, const std::string str_ranges, std::vector<std::string> options) {
    DECLARE_TEST();
    Uri u = uri;

    std::string filename = SSTR(testfile << 1);

    bool noappend = false;
    for(std::vector<std::string>::iterator it = options.begin(); it != options.end(); it++) {
        if(*it == "nomulti") {
            u.addFragmentParam("multirange", "false");
        }
        else if(*it ==  "noappend") {
            noappend = true;
        }
        else if(it->find("nconnections=", 0) == 0) {
            int nconnections = atoi(it->c_str() + 13);
            ASSERT(nconnections > 0, "Unable to parse nconnections");
            u.addFragmentParam("nconnections", SSTR(nconnections));
        }
        else if(it->find("mergewindow=", 0) == 0) {
            int mergewindow = atoi(it->c_str() + 12);
            ASSERT(mergewindow > 0, "Unable to parse mergewindow");
            u.addFragmentParam("mergewindow", SSTR(mergewindow));
        }
        else {
            ASSERT(false, "Unknown option to preadvec: " << *it);
        }
    }

    if(!noappend) {
        u.addPathSegment(filename);
    }

    std::vector<std::string> ranges = split(str_ranges, ",");
    DavIOVecInput inVec[ranges.size()];
    DavIOVecOuput outVec[ranges.size()];

    for(size_t i = 0; i < ranges.size(); i++) {
        std::vector<std::string> parts = split(ranges[i], "-");
        ASSERT(parts.size() == 2, "Cannot parse range");
        dav_off_t start = atoi(parts[0].c_str());
        dav_off_t end = atoi(parts[1].c_str());

        dav_ssize_t size = end - start + 1;

        inVec[i].diov_buffer = new char[size];
        inVec[i].diov_size = size;
        inVec[i].diov_offset = start;

        std::cout << "Adding range: " << start << "-" << end << std::endl;
    }

    Context context;
    DavFile file(context, params, u);
    DavixError *err = NULL;
    file.readPartialBufferVec(&params, inVec, outVec, ranges.size(), &err);

    for(size_t i = 0; i < ranges.size(); i++) {
        std::string chunk( (char*) outVec[i].diov_buffer, outVec[i].diov_size);
        std::cout << "Chunk: " << chunk << std::endl;

        ASSERT(chunk.size() == inVec[i].diov_size, "unexpected chunk size");
        if(filename == SSTR(testfile << 1)) {
            ASSERT(chunk == testString.substr(inVec[i].diov_offset, inVec[i].diov_size), "wrong chunk contents");
        }
    }
    std::cout << "All OK" << std::endl;
}

void detectwebdav(const RequestParams &params, const Uri uri, bool result) {
    DECLARE_TEST();

    Context context;
    DavixError *err = NULL;
    WebdavSupport::Type res = detect_webdav_support(context, params, uri, &err);
    if(result) {
        ASSERT(res == WebdavSupport::YES, "");
    }
    else if(!result) {
        ASSERT(res == WebdavSupport::NO || res == WebdavSupport::UNKNOWN, "");
    }
    else {
      ASSERT(false, "Unknown result");
    }
}

void assert_args(const std::vector<std::string> &cmd, int nargs) {
    ASSERT(cmd.size() != 0, "assert_args called with empty command!");
    ASSERT(cmd.size() == nargs+1, "Wrong number of arguments to " << cmd[0] << ": " << cmd.size()-1 << ", expected: " << nargs);
}

void run(int argc, char** argv) {
    RequestParams params;
    params.setOperationRetry(0);

    std::vector<option::Option> opts = parse_args(argc, argv);
    Auth::Type auth = Auth::fromString(retrieve(opts, Opt::AUTH));

    ASSERT(opts[Opt::COMMAND] != NULL, "--command is necessary");
    ASSERT(opts[Opt::URI] != NULL, "--uri is necessary");
    ASSERT(auth != Auth::ILLEGAL, "--auth is necessary, and can only be one of aws, proxy, azure, none");

    if(opts[Opt::TRACE]) {
        std::string scope = retrieve(opts, Opt::TRACE);
        setLogScope(0);
        setLogScope(scope);
        setLogLevel(DAVIX_LOG_TRACE);
    }

    std::vector<std::string> cmd = split(retrieve(opts, Opt::COMMAND), " ");
    Uri uri = Uri(retrieve(opts, Opt::URI));
    authentication(opts, auth, params);

    if(cmd[0] == "makeCollection") {
        assert_args(cmd, 0);
        makeCollection(params, uri);
    }
    else if(cmd[0] == "populate") {
        assert_args(cmd, 1);
        populate(params, uri, atoi(cmd[1].c_str()));
    }
    else if(cmd[0] == "remove") {
        assert_args(cmd, 0);
        ASSERT(cmd.size() == 1, "Wrong number of arguments to remove");
        remove(params, uri);
    }
    else if(cmd[0] == "listing") {
        assert_args(cmd, 1);
        listing(params, uri, atoi(cmd[1].c_str()));
    }
    else if(cmd[0] == "putMoveDelete") {
        assert_args(cmd, 0);
        ASSERT(cmd.size() == 1, "Wrong number of arguments to putMoveDelete");
        putMoveDelete(params, uri);
    }
    else if(cmd[0] == "depopulate") {
        assert_args(cmd, 1);
        depopulate(params, uri, atoi(cmd[1].c_str()));
    }
    else if(cmd[0] == "countfiles") {
        assert_args(cmd, 1);
        countfiles(params, uri, atoi(cmd[1].c_str()));
    }
    else if(cmd[0] == "statdir") {
        assert_args(cmd, 0);
        statdir(params, uri);
    }
    else if(cmd[0] == "statfile") {
        assert_args(cmd, 0);
        statfile(params, uri);
    }
    else if(cmd[0] == "movefile") {
        assert_args(cmd, 0);
        movefile(params, uri);
    }
    else if(cmd[0] == "preadvec") {
        if(cmd.size() == 2) {
            preadvec(params, uri, cmd[1], std::vector<std::string>());
        }
        else if(cmd.size() == 3) {
            preadvec(params, uri, cmd[1], split(cmd[2], ","));
        }
        else {
            ASSERT(false, "Wrong number of arguments to preadvec");
        }
    }
    else if(cmd[0] == "detectwebdav") {
        assert_args(cmd, 1);
        bool expected = false;
        if(cmd[1] == "1") {
            expected = true;
        }
        else if(cmd[1] == "0") {
            expected = false;
        }
        else {
            ASSERT(false, "Unexpected input for expected result");
        }

        detectwebdav(params, uri, expected);
    }
    else {
        ASSERT(false, "Unknown command: " << cmd[0]);
    }
}

int main(int argc, char** argv) {
    try {
        initialization(argc, argv);
        run(argc, argv);
    }
    catch(std::exception &e) {
        std::cout << e.what() << std::endl;
        return 1;
    }

    return 0;
}
