#include <stdio.h>
#include <iostream>
#include <vector>
#include "optionparser.h"
#include <sstream>
#include <stdexcept>

#include <davix.hpp>
#include "davix_test_lib.h"
#include "utils/davix_s3_utils.hpp"
#include "../TestcaseHandler.hpp"

using namespace Davix;
using std::placeholders::_1;

#define DBG(message) std::cerr << __FILE__ << ":" << __LINE__ << " -- " << #message << " = " << message << std::endl;
#define SSTR(message) static_cast<std::ostringstream&>(std::ostringstream().flush() << message).str()

#include "lorem-ipsum.h" // define std::string teststring
const std::string testfile("davix-testfile-");

#define ASSERT(assertion, msg) \
    if((assertion) == false) throw std::runtime_error( SSTR(__FILE__ << ":" << __LINE__ << " (" << __func__ << "): Assertion " << #assertion << " failed.\n" << msg))

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
enum Type {AWS, PROXY, AZURE, SWIFT, NONE, ILLEGAL};
Type fromString(const std::string &str) {
    if(str == "aws")
        return Auth::AWS;
    if(str == "proxy")
        return Auth::PROXY;
    if(str == "azure")
        return Auth::AZURE;
    if(str == "swift")
        return Auth::SWIFT;
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
                    AZUREKEY, S3ALTERNATE, CERT, URI, TRACE, COMMAND, OSTOKEN, OSPROJECTID, SWIFTACCOUNT};
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
        {Opt::OSTOKEN, 0, "", "ostoken", option_nonempty, "--ostoken Openstack token"},
        {Opt::OSPROJECTID, 0, "", "osprojectid", option_nonempty, "--osprojectid Openstack project id"},
        {Opt::SWIFTACCOUNT, 0, "", "swiftaccount", option_nonempty, "--swiftaccount Swift Account"},
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
    else if(auth == Auth::SWIFT) {
        ASSERT(opts[Opt::OSTOKEN] != NULL, "--ostoken is required when using swift");
        if(opts[Opt::OSPROJECTID] == NULL && opts[Opt::SWIFTACCOUNT] == NULL) {
            std::cout << "--osprojectid or --swiftaccount is required when using swift" << std::endl;
            std::abort();
        }

        params.setProtocol(RequestProtocol::Swift);
        params.setOSToken(retrieve(opts, Opt::OSTOKEN));
        params.setOSProjectID(retrieve(opts, Opt::OSPROJECTID));
        params.setSwiftAccount(retrieve(opts, Opt::SWIFTACCOUNT));
    }
    else {
        ASSERT(false, "unknown authentication method");
    }
}

void remove(TestcaseHandler &handler, const RequestParams &params, const Uri uri) {
    handler.setName(SSTR("Delete " << uri.getString()));

    // a very dangerous test.. Make sure that uri at least
    // contains "davix-test" in its path.
    bool safePath = uri.getPath().find("davix-test") != std::string::npos;
    handler.check(safePath, "Path is safe and contains 'davix-test'");
    if(!safePath) return;

    Context context;

    try {
        DavFile file(context, params, uri);
        file.deletion(&params);
        handler.pass("Deletion successful");
    }
    catch(const DavixException &exc) {
        handler.fail(SSTR("Exception: " << exc.what()));
    }
}

void depopulate(TestcaseHandler &handler, const RequestParams &params, Uri uri, int nfiles) {
    handler.setName(SSTR("Depopulate " << uri.getString() << ", remove " << nfiles << " files"));

    Context context;
    for(int i = 1; i <= nfiles; i++) {
        Uri u(uri);
        u.addPathSegment(SSTR(testfile << i));
        remove(handler.makeChild(), params, u);
    }
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

void statdir(TestcaseHandler &handler, const RequestParams &params, Uri uri) {
    handler.setName(SSTR("Stat on " << uri.getString() << ", ensure is a directory"));

    Context context;
    DavFile file(context, params, uri);
    StatInfo info;
    file.statInfo(&params, info);

    handler.info(SSTR("Mode: " << string_from_mode(info.mode)));
    handler.info(SSTR("Size: " << info.size));

    // http / https will use a HEAD, from which we can't determine if this is
    // a directory or not
    if(uri.getProtocol() != "http" && uri.getProtocol() != "https") {
        handler.check(S_ISDIR(info.mode), "Ensure S_ISDIR shows a directory");
    }
}

void makeCollection(TestcaseHandler &handler, const RequestParams &params, Uri uri) {
    handler.setName(SSTR("Create directory on " << uri.getString()));

    DavixError* err = NULL;

    Context context;
    DavFile file(context, params, uri);
    file.makeCollection(&params, &err);
    if(!handler.checkDavixError(&err)) return;

    // make sure it is empty
    DavFile::Iterator it = file.listCollection(&params);
    handler.check(it.name() == "" && !it.next(), "Ensure newly created directory is empty");

    // do a stat, make sure it's a dir
    statdir(handler.makeChild(), params, uri);

    Uri u2 = uri;
    u2.ensureTrailingSlash();
    statdir(handler.makeChild(), params, u2);
}

#define NEON_S3_SIGN_DURATION 3600

//------------------------------------------------------------------------------
// stat a file through signed URI, make sure it's a file.
//------------------------------------------------------------------------------
void statfileFromSignedURI(TestcaseHandler &handler, const RequestParams &params, const Uri uri) {
  handler.setName(SSTR("Stat file through signed uri on " << uri.getString()));

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

  handler.info(SSTR("Mode: " << string_from_mode(info.mode)));
  handler.info(SSTR("Size: " << info.size));
  handler.check(!S_ISDIR(info.mode), "Ensure S_ISDIR shows a file");
}

//------------------------------------------------------------------------------
// stat a file, make sure it's a file.
//------------------------------------------------------------------------------
void statfile(TestcaseHandler &handler, const RequestParams &params, const Uri uri) {
    handler.setName(SSTR("Stat on " << uri.getString() << ", ensure is a file"));

    Context context;
    DavFile file(context, params, uri);
    StatInfo info;

    try {
        file.statInfo(&params, info);
        handler.info(SSTR("Mode: " << string_from_mode(info.mode)));
        handler.info(SSTR("Size: " << info.size));

        handler.check(!S_ISDIR(info.mode), "Ensure S_ISDIR shows a file");
    }
    catch(const DavixException &exc) {
        handler.fail(SSTR("Exception: " << exc.what()));
    }


    if(!params.getAwsAutorizationKeys().first.empty()) {
      // Now try statting through the signed URL
      statfileFromSignedURI(handler.makeChild(), params, uri);
    }
}

void movefile(TestcaseHandler &handler, const RequestParams &params, const Uri uri) {
    Uri u1(uri);
    Uri u2(uri);

    u1.addPathSegment(SSTR(testfile << 1));
    u2.addPathSegment(SSTR(testfile << 1 << "-moved"));

    handler.setName(SSTR("Move " << u1.getString() << " to " << u2.getString()));

    Context context;

    DavFile source(context, params, u1);
    DavFile dest(context, params, u2);

    try {
        source.move(&params, dest);
        statfile(handler.makeChild(), params, u2);
        dest.move(&params, source);
        handler.pass("Move successful");
    }
    catch(const DavixException &exc) {
        handler.fail(SSTR("Exception: " << exc.what()));
    }
}

void uploadFile(TestcaseHandler &handler, Context &ctx, const RequestParams &params, const Uri uri) {
    handler.setName(SSTR("Upload testfile to " << uri.getString()));

    DavFile file(ctx, params, uri);

    try {
        file.put(NULL, testString.c_str(), testString.size());
        handler.pass(SSTR("File " << uri.getString() << " uploaded."));
    }
    catch(const DavixException &err) {
        handler.fail(SSTR("Exception: " << err.what()));
    }

    statfile(handler, params, uri);
}

void populate(TestcaseHandler &handler, const RequestParams &params, const Uri uri, const int nfiles) {
    handler.setName(SSTR("Populate " << uri.getString() << " with " << nfiles << " files"));

    Context context;
    for(int i = 1; i <= nfiles; i++) {
        Uri u(uri);
        u.addPathSegment(SSTR(testfile << i));
        uploadFile(handler.makeChild(), context, params, u);
    }
}

// count the number of files in folder
void countfiles(TestcaseHandler &handler, const RequestParams &params, const Uri uri, const int nfiles) {
    handler.setName(SSTR("List " << uri.getString() << ", expect " << nfiles << " files"));

    try {
        Context context;
        DavFile file(context, params, uri);
        DavFile::Iterator it = file.listCollection(&params);
        int i = 0;

        do {
            // workaround for Swift, discard the first entry which is the directory created before
            if(params.getProtocol() == RequestProtocol::Swift && i == 0){
                it.next();
            }
            i++;
        } while(it.next());

        handler.check(i == nfiles, SSTR("Directory contains " << nfiles << " files"));
    }
    catch(const DavixException &exc) {
        handler.fail(SSTR("Exception: " << exc.what()));
    }
}

// confirm that the files listed are the exact same ones uploaded during a populate test
void listing(TestcaseHandler &handler, const RequestParams &params, const Uri uri, const int nfiles) {
    handler.setName(SSTR("List " << uri.getString() << ", expect " << nfiles << " files"));

    try {
        int hits[nfiles+1];
        for(int i = 0; i <= nfiles; i++) hits[i] = 0;

        Context context;
        DavFile file(context, params, uri);
        DavFile::Iterator it = file.listCollection(&params);

        int i = 0;
        do {
            // workaround for Swift, discard the first entry which is the directory created before
            if(params.getProtocol() == RequestProtocol::Swift && i == 0){
                it.next();
            }
            i++;
            std::string name = it.name();

            // make sure the filenames are the same as the ones we uploaded
            if(name.size() <= testfile.size()) {
                handler.fail(SSTR("Unexpected filename: " << name));
                return;
            }

            std::string part1 = name.substr(0, testfile.size());
            std::string part2 = name.substr(testfile.size(), name.size()-1);

            if(part1 != testfile) {
                handler.fail(SSTR("Unexpected filename: " << part1));
                return;
            }

            int num = atoi(part2.c_str());
            if(num <= 0 || num > nfiles) {
                handler.fail(SSTR("Unexpected file number: " << num));
                return;
            }

            hits[num]++;
        } while(it.next());

        handler.check(i == nfiles, SSTR("Ensure directory contains " << nfiles << " files"));

        // count all hits to make sure all have exactly one
        for(int i = 1; i <= nfiles; i++) {
            handler.check(hits[i] == 1, SSTR("Ensure testfile #" << i << " is found"));
        }
    }
    catch(const DavixException &exc) {
        handler.fail(SSTR("Exception: " << exc.what()));
    }
}

void checksum(TestcaseHandler &handler, const RequestParams &params, const Uri uri, const std::string chk_algo) {
    handler.setName(SSTR("Get checksum of file: " << uri.getString()));

    try {
        Context context;
        DavFile file(context, params, uri);

        std::string chk;
        DavixError* err = NULL;
        file.checksum(&params, chk, chk_algo, &err);
        handler.pass(SSTR("File " << chk_algo << " checksum is " << chk));
    } catch (const DavixException &exc) {
        handler.fail(SSTR("Exception: " << exc.what()));
    }
}

/* upload a file and move it around */
void putMoveDelete(TestcaseHandler &handler, const RequestParams &params, const Uri uri) {
    handler.setName(SSTR("Put-move-delete on " << uri.getString()));

    Uri u = uri;
    Uri u2 = uri;
    u.addPathSegment(SSTR(testfile << "put-move-delete"));
    u2.addPathSegment(SSTR(testfile << "put-move-delete-MOVED"));

    Context context;
    DavFile file(context, params, u);
    file.put(&params, testString.c_str(), testString.size());
    handler.pass(SSTR("Put on " << u.getString()));

    DavFile movedFile(context, params, u2);
    file.move(&params, movedFile);
    handler.pass(SSTR("Move on " << u2.getString()));

    movedFile.deletion(&params);
    handler.pass(SSTR("Delete on " << u2.getString()));
}

//------------------------------------------------------------------------------
// Concatenate vector into a string
//------------------------------------------------------------------------------
static std::string vecToString(const std::vector<std::string> &vec, const std::string &delim) {
    std::ostringstream ss;
    for(size_t i = 0; i < vec.size(); i++) {
        ss << vec[i];

        if(i != vec.size() - 1) {
            ss << delim;
        }
    }

    return ss.str();
}

void preadvec(TestcaseHandler &handler, const RequestParams &params, const Uri uri, const std::string str_ranges, std::vector<std::string> options) {
    handler.setName(SSTR("preadvec on " << uri.getString()));
    handler.info(SSTR("Options: " << str_ranges << " | " << vecToString(options, ",")));

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

            if(nconnections <= 0) {
                handler.fail("Unable to parse nconnections");
                return;
            }

            u.addFragmentParam("nconnections", SSTR(nconnections));
        }
        else if(it->find("mergewindow=", 0) == 0) {
            int mergewindow = atoi(it->c_str() + 12);

            if(mergewindow <= 0) {
                handler.fail("Unable to parse mergewindow");
                return;
            }

            u.addFragmentParam("mergewindow", SSTR(mergewindow));
        }
        else {
            handler.fail(SSTR("Unknown option to preadvec: " << *it));
            return;
        }
    }

    if(!noappend) {
        u.addPathSegment(filename);
    }

    std::vector<std::string> ranges = split(str_ranges, ",");
    DavIOVecInput inVec[ranges.size()];
    DavIOVecOuput outVec[ranges.size()];

    std::vector<std::unique_ptr<std::string>> buffers;

    for(size_t i = 0; i < ranges.size(); i++) {
        std::vector<std::string> parts = split(ranges[i], "-");

        if(parts.size() != 2) {
            handler.fail(SSTR("Cannot parse range: " << ranges[i]));
            return;
        }

        dav_off_t start = atoi(parts[0].c_str());
        dav_off_t end = atoi(parts[1].c_str());

        dav_ssize_t size = end - start + 1;

        buffers.emplace_back(new std::string(size, ' '));
        inVec[i].diov_buffer = (void*) buffers.back()->c_str(); // new char[size];
        inVec[i].diov_size = size;
        inVec[i].diov_offset = start;
    }

    Context context;
    DavFile file(context, params, u);
    DavixError *err = NULL;
    file.readPartialBufferVec(&params, inVec, outVec, ranges.size(), &err);
    if(!handler.checkDavixError(&err)) return;

    for(size_t i = 0; i < ranges.size(); i++) {
        std::string chunk( (char*) outVec[i].diov_buffer, outVec[i].diov_size);

        handler.check(chunk.size() == inVec[i].diov_size, SSTR("Validate chunk #" << i << " size"));

        if(filename == SSTR(testfile << 1)) {
            handler.check(chunk == testString.substr(inVec[i].diov_offset, inVec[i].diov_size), SSTR("Validate chunk #" << i << " contents"));
        }
    }
}

void preadvec_all_opts(TestcaseHandler &handler, const RequestParams &params, const Uri uri, const std::string str_ranges) {
    handler.setName(SSTR("preadvec-all-opts on " << uri.getString() << ": " << str_ranges));

    preadvec(handler.makeChild(), params, uri, str_ranges, split("mergewindow=1", ","));
    preadvec(handler.makeChild(), params, uri, str_ranges, split("mergewindow=1000", ","));
    preadvec(handler.makeChild(), params, uri, str_ranges, split("nomulti,mergewindow=1", ","));
    preadvec(handler.makeChild(), params, uri, str_ranges, split("nomulti,mergewindow=1", ","));
}

void preadvec_all(TestcaseHandler &handler, const RequestParams &params, const Uri uri) {
    handler.setName(SSTR("preadvec-all on " << uri.getString()));

    preadvec_all_opts(handler.makeChild(), params, uri, "0-10,5-10,3-4,30-40,200-305,1000-1500");
    preadvec_all_opts(handler.makeChild(), params, uri, "0-10,5-10");
    preadvec_all_opts(handler.makeChild(), params, uri, "0-10,15-20");
}

void detectwebdav(TestcaseHandler &handler, const RequestParams &params, const Uri uri, bool result) {
    handler.setName(SSTR("Detect WebDAV support on " << uri.getString() << ", expect " << result));

    Context context;
    DavixError *err = NULL;
    WebdavSupport::Type res = detect_webdav_support(context, params, uri, &err);

    if(result) {
        handler.check(res == WebdavSupport::YES, "Ensure endpoint has WebDAV support");
        ASSERT(res == WebdavSupport::YES, "");
    }
    else if(!result) {
        handler.check(res == WebdavSupport::NO || res == WebdavSupport::UNKNOWN, "Ensure endpoint does not have WebDAV support");
    }
    else {
        handler.fail("Unknown failure");
    }
}

void assert_args(const std::vector<std::string> &cmd, int nargs) {
    ASSERT(cmd.size() != 0, "assert_args called with empty command!");
    ASSERT(cmd.size() == nargs+1, "Wrong number of arguments to " << cmd[0] << ": " << cmd.size()-1 << ", expected: " << nargs);
}

bool run(int argc, char** argv) {
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

    TestcaseHandler handler;

    if(cmd[0] == "makeCollection") {
        assert_args(cmd, 0);
        makeCollection(handler, params, uri);
    }
    else if(cmd[0] == "populate") {
        assert_args(cmd, 1);
        populate(handler, params, uri, atoi(cmd[1].c_str()));
    }
    else if(cmd[0] == "checksum") {
        assert_args(cmd, 1);
        checksum(handler, params, uri, cmd[1].c_str());
    }
    else if(cmd[0] == "remove") {
        assert_args(cmd, 0);
        ASSERT(cmd.size() == 1, "Wrong number of arguments to remove");
        remove(handler, params, uri);
    }
    else if(cmd[0] == "listing") {
        assert_args(cmd, 1);
        listing(handler, params, uri, atoi(cmd[1].c_str()));
    }
    else if(cmd[0] == "putMoveDelete") {
        assert_args(cmd, 0);
        ASSERT(cmd.size() == 1, "Wrong number of arguments to putMoveDelete");
        putMoveDelete(handler, params, uri);
    }
    else if(cmd[0] == "depopulate") {
        assert_args(cmd, 1);
        depopulate(handler, params, uri, atoi(cmd[1].c_str()));
    }
    else if(cmd[0] == "countfiles") {
        assert_args(cmd, 1);
        countfiles(handler, params, uri, atoi(cmd[1].c_str()));
    }
    else if(cmd[0] == "statdir") {
        assert_args(cmd, 0);
        statdir(handler, params, uri);
    }
    else if(cmd[0] == "statfile") {
        assert_args(cmd, 0);
        statfile(handler, params, uri);
    }
    else if(cmd[0] == "movefile") {
        assert_args(cmd, 0);
        movefile(handler, params, uri);
    }
    else if(cmd[0] == "preadvec") {
        if(cmd.size() == 2) {
            preadvec(handler, params, uri, cmd[1], std::vector<std::string>());
        }
        else if(cmd.size() == 3) {
            preadvec(handler, params, uri, cmd[1], split(cmd[2], ","));
        }
        else {
            handler.fail("Wrong number of arguments to preadvec");
        }
    }
    else if(cmd[0] == "preadvec-all-opts") {
        assert_args(cmd, 1);
        preadvec_all_opts(handler, params, uri, cmd[1]);
    }
    else if(cmd[0] == "preadvec-all") {
        assert_args(cmd, 0);
        preadvec_all(handler, params, uri);
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

        detectwebdav(handler, params, uri, expected);
    }
    else if(cmd[0] == "fulltest") {
        assert_args(cmd, 0);

        handler.setName(SSTR("Full-test on " << uri.getString()));

        makeCollection(handler.makeChild(), params, uri);
        populate(handler.makeChild(), params, uri, 5);
        listing(handler.makeChild(), params, uri, 5);
        preadvec_all(handler.makeChild(), params, uri);
        movefile(handler.makeChild(), params, uri);
        depopulate(handler.makeChild(), params, uri, 3);
        countfiles(handler.makeChild(), params, uri, 2);
        remove(handler.makeChild(), params, uri);
    }
    else if(cmd[0] == "fulltest-s3") {
        assert_args(cmd, 0);

        handler.setName(SSTR("Full-test S3 on " << uri.getString()));

        makeCollection(handler.makeChild(), params, uri);
        populate(handler.makeChild(), params, uri, 5);
        listing(handler.makeChild(), params, uri, 5);
        preadvec_all(handler.makeChild(), params, uri);
        movefile(handler.makeChild(), params, uri);
        depopulate(handler.makeChild(), params, uri, 3);
        countfiles(handler.makeChild(), params, uri, 2);
        depopulate(handler.makeChild(), params, uri, 5);
    }
    else if(cmd[0] == "fulltest-azure") {
        assert_args(cmd, 0);

        handler.setName(SSTR("Full-test Azure on " << uri.getString()));

        populate(handler.makeChild(), params, uri, 5);
        statdir(handler.makeChild(), params, uri);
        listing(handler.makeChild(), params, uri, 5);
        preadvec_all(handler.makeChild(), params, uri);
        depopulate(handler.makeChild(), params, uri, 5);
    }
    else if(cmd[0] == "fulltest-swift") {
        assert_args(cmd, 0);

        handler.setName(SSTR("Full-test Swift on " << uri.getString()));

        makeCollection(handler.makeChild(), params, uri);
        populate(handler.makeChild(), params, uri, 5);
        listing(handler.makeChild(), params, uri, 5);
        preadvec_all(handler.makeChild(), params, uri);
        movefile(handler.makeChild(), params, uri);
        depopulate(handler.makeChild(), params, uri, 3);
        countfiles(handler.makeChild(), params, uri, 2);
        depopulate(handler.makeChild(), params, uri, 5);
    }
    else {
        ASSERT(false, "Unknown command: " << cmd[0]);
    }

    return handler.ok();
}

int main(int argc, char** argv) {
    bool success = false;
    try {
        success = run(argc, argv);
    }
    catch(std::exception &e) {
        std::cout << e.what() << std::endl;
        return 1;
    }

    if(success) {
        return 0;
    }

    return 1;
}
