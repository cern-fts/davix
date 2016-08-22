#include <davix.hpp>
#include <string>
#include <iostream>
#include <fstream>
#include <sys/time.h>
#include <stdio.h>
#include <vector>
#include <sstream>
#include <pthread.h>
#include "chunk_queue.h"

#define BUFFER_SIZE 100   // in MB
#define MAX_READ_PER_LOOP 20480

using namespace Davix;
using namespace std;

// command line options
struct Options
{
    RequestParams params;
    char mode;
    int vec_size;
    int no_of_thread;
    bool check;
    bool debug;
    bool silent;
    bool hasinputfile;
    std::vector<std::string> vec_arg;
    std::string inputfile;

    Options() :
        params(),
        mode(),
        vec_size(),
        no_of_thread(),
        check(false),
        debug(false),
        silent(false),
        hasinputfile(false),
        vec_arg(),
        inputfile()
    {
    }
};

// producer thread arguments
struct ProducerArgs
{
    long* length;
    long* offset;
    int nread;
    DAVIX_FD* fd;
    ChunkQueue* cq;
};

// reader threads arguments
struct ReaderArgs
{
    DAVIX_FD* fd;
    ChunkQueue* cq;
    struct Options* opts;
    DavPosix* infile;
    bool* iserror;
    long* totalreadscount;
    long long* totalbytesread;
    pthread_mutex_t* mutex;
};


int ParseOptions(int argc, char* argv[], Options & p);
void PrintUsage();
int ReadSome(long *offs, long *lens, int maxnread, long long &totalbytes, bool &last_iter, Options &p, ifstream& in_file);
void errorPrint(DavixError ** err);
void* ThreadRead(void* args);
void* PopulateQueue(void* args);


int main(int argc, char* argv[])
{
    void *buffer = NULL;
    timeval tv;
    double starttime = 0, openphasetime = 0, endtime = 0, closetime = 0;
    Options opts;
    DAVIX_FD* fd;
    Context context;
    DavixError* tmp_err = NULL;
    string summarypref = "$$$";
    string opt_type = "read";
    string filepath;
    bool iserror = false;
    bool last_batch = false;
    DavPosix* infile = new DavPosix(&context);
    std::vector<std::string> filename;
    std::vector<DAVIX_FD *> davfd_vec;
    std::vector<int> fd_vec;
    std::vector<DavFile> dfile_vec;
    std::vector<DavixError> DavErr_vec;
    int file_count = 0;
    long long totalbytesread = 0, prevtotalbytesread = 0, totalbytestoprocess = 0;
    long totalreadscount = 0, totalwritecount = 0, totalbyteswritten = 0;
    int ret = -1;

    ChunkQueue cq;

    // enable grid mode
    context.loadModule("grid");

    gettimeofday(&tv, 0);
    starttime = tv.tv_sec + tv.tv_usec / 1000000.0;
    closetime = openphasetime = starttime;

    ParseOptions(argc, argv, opts);
    buffer = malloc(BUFFER_SIZE*1024*1024);

    cout << endl;

    bool isURL = !(opts.vec_arg[0].find("http") == string::npos);

    if(isURL)
    {
        switch(opts.mode)
        {
            case 'r':
            case 't':
            case 'v':
                {
                    cout << "Opening - " << opts.vec_arg[0] << " --------------------- ";

                    if((fd = infile->open(NULL, opts.vec_arg[0], O_RDONLY, &tmp_err)) != NULL)
                    {
                        cout << "success" << endl;
                        davfd_vec.push_back(fd);
                        filename.push_back(opts.vec_arg[0]);
                        file_count++;
                    }
                    else
                    {
                        cout << "Failed" << endl << endl;
                        if(tmp_err != NULL)
                        {
                            DavErr_vec.push_back(*tmp_err);
                            errorPrint(&tmp_err);
                        }
                        iserror = true;
                    }
                    break;
                }

            case 'w':
                {
                    cout << "Creating Davix file - " << opts.vec_arg[0] << endl;
                    DavFile f(context, opts.vec_arg[0]);
                    dfile_vec.push_back(f);
                    filename.push_back(opts.vec_arg[0]);
                    file_count++;
                    break;
                }

            default:
                {
                    cerr << endl << "Invaild option." << endl;
                    PrintUsage();
                    exit(-1);
                }
        } // switch
    } // isURL
    else //isfile
    {
        ifstream files(opts.vec_arg[0].c_str() );
        files >> filepath;

        switch(opts.mode)
        {
            case 'r':
            case 't':
            case 'v':
                {
                    while(!files.eof() && files.good() )
                    {
                        if(!filepath.empty() )
                        {
                            cout << "Opening - " << filepath << " --------------------- ";

                            if((fd = infile->open(NULL, filepath, O_RDONLY, &tmp_err)) != NULL)
                            {
                                cout << "Success" << endl;
                                davfd_vec.push_back(fd);
                                filename.push_back(filepath);
                                file_count++;
                            }
                            else
                            {
                                cout << "Failed" << endl << endl;
                                if(tmp_err != NULL)
                                {
                                    DavErr_vec.push_back(*tmp_err);
                                    errorPrint(&tmp_err);
                                }
                                iserror = true;
                            }
                        }
                        files >> filepath;
                    } // while file
                    break;
                }

            case 'w':
                {
                    if(!files.good() ) // empty, file is write target
                    {
                        cout << "Creating Davix file - " << opts.vec_arg[0] << endl;
                        DavFile f(context, opts.vec_arg[0]);
                        dfile_vec.push_back(f);
                        filename.push_back(filepath);
                        file_count++;
                    }
                    else // file has content, read them
                    {
                        while(!files.eof() && files.good() )
                        {
                            if(!filepath.empty() )
                            {
                                cout << "Creating Davix file - " << filepath << endl;
                                DavFile f(context, filepath);
                                dfile_vec.push_back(f);
                                filename.push_back(filepath);
                                file_count++;
                            }
                            files >> filepath;
                        } // while
                    }
                    break;
                }

            default:
                {
                    cerr << endl << "Invaild option." << endl;
                    PrintUsage();
                    exit(-1);
                }
        } // switch
    } // isfile

    gettimeofday(&tv, 0);
    openphasetime = tv.tv_sec + tv.tv_usec / 1000000.0;

    cout << endl;

    long long bytes_read = 0;
    int ntoread = 0;
    int maxtoread = MAX_READ_PER_LOOP;
    long v_offsets[MAX_READ_PER_LOOP];
    long v_lens[MAX_READ_PER_LOOP];
    DavIOVecInput inVec[opts.vec_size];
    DavIOVecOuput outVec[opts.vec_size];

    // Davix doesn't support remote pwrite yet, write to tmp file before uploading to simulate the effect
    std::FILE* tmpf = std::tmpfile();
    int fdd = fileno(tmpf);

    ifstream input;

    if(opts.hasinputfile == true)
    {
        input.open(opts.inputfile.c_str(),ios::in);
        if(!input)
        {
            std::cerr << endl << "Cannot open input file.";
            return -1;
        }
    }

    while((ntoread = ReadSome(v_offsets, v_lens, maxtoread, totalbytestoprocess, last_batch, opts, input)))
    {
        cout << ".";

        switch(opts.mode)
        {
            case 'r':   // non-vectored read
                {
                    for(int i = 0; i < file_count; ++i)
                    {
                        cout << endl << "Reading - " << filename[i] << endl << endl;

                        for(int j = 0; j < ntoread; j++)
                        {
                            bytes_read = infile->pread(davfd_vec[i], buffer, v_lens[j], v_offsets[j], &tmp_err);

                            if(tmp_err != NULL)
                            {
                                DavErr_vec.push_back(*tmp_err);
                                errorPrint(&tmp_err);
                                iserror = true;
                                break;
                            }
                            if(bytes_read <= 0)
                            {
                                cerr << "---Read (" << j+1 << " of " << ntoread << ") " <<
                                    v_lens[j] << "@" << v_offsets[j] <<
                                    " returned " << bytes_read << endl;

                                iserror = true;
                                break;
                            }

                            if(!opts.silent)
                            {
                                printf("%20s %-20ld", "Offset: ", v_offsets[j]);
                                printf("%s %-20ld", "Length: ", v_lens[j]);
                                printf("%s %-20lld\n", "Read: ", bytes_read);
                            }

                            totalbytesread += bytes_read;
                            totalreadscount++;

                            // do byte check here
                            if(opts.check == true)
                            {
                                for(int k = 0; k < v_lens[j]; k++)
                                {
                                    if(((k + v_offsets[j]) % 256) != ((unsigned char *)buffer)[k])
                                    {
                                        cerr << "%Byte check -- Error in the file offset: " << k + v_offsets <<
                                        " buffer: " << (int)((unsigned char *)buffer)[k] << " expected: " << (k+v_offsets[j]) % 256 << endl;
                                        iserror = true;
                                        break;
                                    }
                                }
                                if(!iserror && !opts.silent)
                                {
                                    cout << "$Byte check -- Passed." << endl;
                                }
                            }

                        }
                    }
                    break;
                }

            case 't':   // threaded read
                {
                    for(int i = 0; i < file_count; ++i)
                    {
                        cq.SetQueueState(STARTED);
                        pthread_mutex_t mutex;

                        cout << endl << "Reading - " << filename[i] << endl << endl;

                        pthread_mutex_init(&mutex,0);

                        // populate producer args
                        ProducerArgs tk_args;
                        tk_args.length = v_lens;
                        tk_args.offset = v_offsets;
                        tk_args.fd = davfd_vec[i];
                        tk_args.cq = &cq;
                        tk_args.nread = ntoread;

                        pthread_t producer;
                        pthread_create(&producer, NULL, PopulateQueue, &tk_args);

                        // populate reader args
                        ReaderArgs r_args;
                        r_args.fd = davfd_vec[i];
                        r_args.cq = &cq;
                        r_args.opts = &opts;
                        r_args.infile = infile;
                        r_args.iserror = &iserror;
                        r_args.totalreadscount = &totalreadscount;
                        r_args.totalbytesread = &totalbytesread;
                        r_args.mutex = &mutex;

                        pthread_t read_thread[opts.no_of_thread];

                        for(int ii = 0; ii < opts.no_of_thread; ++ii)
                        {
                            pthread_create(&read_thread[ii], NULL, ThreadRead, &r_args);
                        }

                        for(int j = 0; j < opts.no_of_thread; ++j)
                        {
                            pthread_join(read_thread[j], NULL);
                            cq.StopThreads();
                        }
                        pthread_join(producer, NULL);
                        pthread_mutex_destroy(&mutex);
                    }
                    break;
                }

            case 'v':   // vectored read
                for(int i = 0; i < file_count; ++i)
                {
                    cout << endl << "Reading - " << filename[i] << endl;
                    cout << "Vector size - " << opts.vec_size << endl << endl;

                    int no_of_input = ntoread;
                    int vec_size = 0;

                    for(int j = 0; j < ntoread;)
                    {
                        if(no_of_input > opts.vec_size)
                        {
                            vec_size = opts.vec_size;
                        }
                        else
                        {
                            vec_size = no_of_input;
                        }

                        for(int k = 0; k < vec_size; k++)
                        {
                            if( (k+j) >= MAX_READ_PER_LOOP)
                            {
                               break;
                            }
                            inVec[k].diov_offset = v_offsets[k+j];
                            inVec[k].diov_size = v_lens[k+j];
                            inVec[k].diov_buffer = buffer;

                            if(!opts.silent)
                            {
                                printf("%20s %-20ld", "Offset: ", v_offsets[k+j]);
                                printf("%s %-20ld\n", "Length: ", v_lens[k+j]);
                            }
                        }

                        bytes_read = infile->preadVec(davfd_vec[i], inVec, outVec, vec_size, &tmp_err);

                        if(tmp_err != NULL)
                        {
                            DavErr_vec.push_back(*tmp_err);
                            errorPrint(&tmp_err);
                            iserror = true;
                            break;
                        }
                        if(bytes_read <= 0)
                        {
                            cerr << "---Read (" << j+1 << " of " << ntoread << ") " <<
                                v_lens[j] << "@" << v_offsets[j] <<
                                " returned " << bytes_read << endl;

                            iserror = true;
                            break;
                        }

                        if(!opts.silent)
                        {
                            printf("\n%20s %-20lld\n\n", "Read: ", bytes_read);
                        }

                        totalbytesread += bytes_read;
                        totalreadscount++;

                        no_of_input -= opts.vec_size;
                        j += opts.vec_size;
                    }
                }
                break;

            case 'w':   // write
                {
                    for(int i = 0; i < file_count; ++i)
                    {
                        // if file already exists, delete it.
                        dfile_vec[i].deletion(&opts.params, NULL);

                        int retval = 0;

                        DavFile df_tmp(context, filename[i]);

                        for(int j = 0; j < ntoread; j++)
                        {
                            for(int k = 0; k < v_lens[j]; k++)
                            {
                                ((unsigned char *)buffer)[k] = (v_offsets[j]+k) % 256;
                            }
                            ret = pwrite(fdd, buffer, v_lens[j], v_offsets[j]);

                            if (ret <= 0)
                            {
                                cout << endl << "---Write (" << j+1 << " of " << ntoread << ") " <<
                                    v_lens[j] << "@" << v_offsets[j] <<
                                    " returned " << ret << endl;
                                iserror = true;
                                break;
                            }

                            if(retval != 0)
                            {
                                cerr << "DavFile write error: putFromFd" << endl;
                                exit(-1);
                            }

                            if(!opts.silent)
                            {
                                printf("%20s %-20ld", "Offset: ", v_offsets[j]);
                                printf("%s %-20ld\n", "Length: ", v_lens[j]);
                            }
                        }
                        fseek(tmpf,0,SEEK_END);
                        int size = ftell(tmpf);

                        // upload only if this is the last iteration of the input loop
                        if(last_batch == true)
                        {
                            retval = df_tmp.putFromFd(&opts.params, fdd, size, &tmp_err);
                            if(tmp_err != NULL)
                            {
                                DavErr_vec.push_back(*tmp_err);
                                errorPrint(&tmp_err);
                                iserror = true;
                                continue;
                            }

                            totalbyteswritten += size;
                            totalwritecount++;
                            cout << endl << "Write run completed on " << filename[i] << endl << endl;

                        }
                    }
                    break;
                }

            default:
                {
                    cerr << endl << "Option not reconised." << endl;
                    PrintUsage();
                    exit(-1);
                    break;
                }
        }// switch

        if (iserror && prevtotalbytesread)
        {
            totalbytesread = prevtotalbytesread;
            break;
        }

        prevtotalbytesread = totalbytesread;

    }// while ReadSome

    gettimeofday(&tv, 0);
    closetime = tv.tv_sec + tv.tv_usec / 1000000.0;

    cout << endl << "--- Closing all files." << endl;

    for(unsigned int i = 0; i < davfd_vec.size(); ++i)
    {
        infile->close(davfd_vec[i], &tmp_err);
    }

    close(fdd);
    delete(infile);

    cout << "--- Clearing pointer vector." << endl;
    davfd_vec.clear();

    cout << "--- Freeing buffer." << endl;
    free(buffer);

    gettimeofday(&tv, 0);
    endtime = tv.tv_sec + tv.tv_usec / 1000000.0;

    if (iserror) summarypref = "%%%";

    cout << fixed;
    cout << endl << "Summary ----------------------------" << endl;
    cout << summarypref << " Start time: " << starttime << endl;
    cout << summarypref << " Last open time: " << openphasetime << endl;
    cout << summarypref << " Close time: " << closetime << endl;
    cout << summarypref << " End time: " << endtime << endl;
    cout << summarypref << " Open elapsed: " << openphasetime - starttime << endl;
    cout << summarypref << " Data transfer elapsed: " << closetime - openphasetime << endl;
    cout << summarypref << " Close elapsed: " << endtime - closetime << endl;
    cout << summarypref << " Total elapsed: " << endtime - starttime << endl;

    if(opts.mode == 'w')
    {
        cout << summarypref << " Total bytes written: " << totalbyteswritten << endl;
        cout << summarypref << " Max bytes written per sec: " << totalbyteswritten / (closetime - openphasetime) << endl;
        cout << summarypref << " Effective bytes written per sec: " << totalbyteswritten / (endtime - starttime) << endl;
        cout << summarypref << " Write count: " << totalwritecount << endl;
    }
    else
    {
        cout << summarypref << " Total bytes to process: " << totalbytestoprocess * file_count << endl;
        cout << summarypref << " Total bytes read: " << totalbytesread << endl;
        cout << summarypref << " Max bytes read per sec: " << totalbytesread / (closetime - openphasetime) << endl;
        cout << summarypref << " Effective bytes read per sec: " << totalbytesread / (endtime - starttime) << endl;
        cout << summarypref << " Successful read count: " << totalreadscount << endl;

        switch(opts.mode)
        {
            case 'v':
                cout << summarypref << " Vector size: " << opts.vec_size << endl;
                break;

            case 't':
                cout << summarypref << " Number of thread(s): " << opts.no_of_thread << endl;
                break;
        }
    }
    cout << summarypref << " Opened file count: " << file_count << endl;
    cout << endl;

    // print out any unique ocurrence of DavixError
    if(!DavErr_vec.empty() )
    {
        int err_code = 0;
        std::vector<int> err_code_vec;
        err_code = DavErr_vec[0].getStatus();
        err_code_vec.push_back(err_code);
        std::cerr << "Total DavixError: " << DavErr_vec.size() << endl << "Printing error type(s) -" << endl;
        std::cerr << "("<<  DavErr_vec[0].getErrScope() <<") Error: "<< DavErr_vec[0].getErrMsg() << std::endl << std::endl;

        for(unsigned int z = 1; z < DavErr_vec.size(); ++z)
        {
            err_code = DavErr_vec[z].getStatus();

            if (std::find(err_code_vec.begin(), err_code_vec.end(), err_code) == err_code_vec.end())
            {
                err_code_vec.push_back(err_code);
                std::cerr << "("<<  DavErr_vec[z].getErrScope() <<") Error: "<< DavErr_vec[z].getErrMsg() << std::endl << std::endl;
            }
        }
        DavErr_vec.clear();
    }
    return 0;
}// end of main



//===================================================================================================================
//==================================================FUNCTIONS========================================================
//===================================================================================================================




void errorPrint(DavixError ** err)
{
    if(err && *err){
        std::cerr << "("<< (*err)->getErrScope() <<") Error: "<< (*err)->getErrMsg() << std::endl << std::endl;
        DavixError::clearError(err);
    }
}




int ReadSome(long *offs, long *lens, int maxnread, long long &totalbytes, bool &last_iter, Options &p, ifstream& in_file)
{
    if(p.hasinputfile)
    {
        for (int i = 0; i < maxnread;)
        {
            lens[i] = -1;
            offs[i] = -1;

            if (in_file.eof())
            {
                last_iter = true;
                return i;
            }

            in_file >> lens[i] >> offs[i];

            if(lens[i] > (BUFFER_SIZE*1024*1024) )
            {
                std::cerr << endl << "Length exceeds buffer size @ length: " << lens[i] << ", buffer: " << BUFFER_SIZE*1024*1024 << endl << endl;
                exit(-1);
            }

            if ((lens[i] > 0) && (offs[i] >= 0))
            {
                totalbytes += lens[i];
                i++;
            }
        }
        return maxnread;

    }
    else
    {
        for (int i = 0; i < maxnread;)
        {
            lens[i] = -1;
            offs[i] = -1;

            if (cin.eof())
            {
                last_iter = true;
                return i;
            }

            cin >> lens[i] >> offs[i];

            if(lens[i] > (BUFFER_SIZE*1024*1024) )
            {
                std::cerr << endl << "Length exceeds buffer size @ length: " << lens[i] << ", buffer: " << BUFFER_SIZE*1024*1024 << endl << endl;
                exit(-1);
            }

            if ((lens[i] > 0) && (offs[i] >= 0))
            {
                totalbytes += lens[i];
                i++;
            }
        }
        return maxnread;
    }
}




int ParseOptions(int argc, char* argv[], Options & p)
{
    const std::string arg_tool_main= "srwdchv:t:i:";

    int ret = 0;

    if(argc < 2)
    {
        PrintUsage();
        exit(-1);
    }

    while((ret = getopt(argc, argv, arg_tool_main.c_str() )) > 0)
    {
        switch(ret)
        {
            case 'r':
                p.mode = ret;
                break;

            case 'v':
                p.mode = ret;
                p.vec_size = atoi(optarg);
                if(p.vec_size <= 0)
                {
                    std::cerr << endl << "Vector size must be a positive integer." << endl << endl;
                    exit(-1);
                }
                break;

            case 't':
                p.mode = ret;
                p.no_of_thread = atoi(optarg);
                if(p.no_of_thread <= 0)
                {
                    std::cerr << endl << "Number of threads must be a positive integer." << endl << endl;
                    exit(-1);
                }

            case 'w':
                p.mode = ret;
                break;

            case 'h':
                PrintUsage();
                exit(0);
                break;

            case 'c':
                p.check = true;
                break;

            case 'd':
                p.debug = true;
                davix_set_log_level(15);
                break;

            case 's':
                p.silent = true;
                break;

            case 'i':
                p.hasinputfile = true;
                p.inputfile = optarg;
                break;

            default:
                PrintUsage();
                exit(-1);
        }// switch
    }// while getopt

    ret = -1;

    for(int i = optind; i < argc; ++i)
    {
        p.vec_arg.push_back(argv[i]);
        ret = 0;
    }

    if(ret != 0 && p.mode != 'h')
    {
        PrintUsage();
        exit(-1);
    }

    return ret;
}




void* PopulateQueue(void* args)
{
    ProducerArgs* tk = static_cast<ProducerArgs*>(args);

    for(int i = 0; i < tk->nread; ++i)
    {
        tk->cq->pushOp(tk->length[i], tk->offset[i], tk->fd);
    }

    while(true)
    {
        if(tk->cq->GetQueueSize() == 0)
        {
            // signal all workers to exit
            tk->cq->StopThreads();
            break;
        }
    }
    return 0;
}




void* ThreadRead(void* args)
{
    long long bytes_read = 0;
    void *buffer = NULL;
    DavixError* tmp_err = NULL;
    buffer = malloc(BUFFER_SIZE*1024*1024);

    ReaderArgs* rd = static_cast<ReaderArgs*>(args);

    while( rd->cq->GetQueueState() != STOPPED)
    {
        ChunkQueue::worktoken* tk = rd->cq->getOp();

        bytes_read = rd->infile->pread(rd->fd, buffer, tk->length, tk->offset, &tmp_err);

        if(!rd->opts->silent)
        {
            printf("%20s %-20ld", "Offset: ", tk->offset);
            printf("%s %-20ld", "Length: ", tk->length);
            printf("%s %-20lld\n", "Read: ", bytes_read);
        }

        if (bytes_read <= 0)
        {
            cout << "---Read " <<
                tk->length << "@" << tk->offset <<
                " returned " << bytes_read << endl;

            *rd->iserror = true;
            errorPrint(&tmp_err);
            delete(tk);
            continue;
        }

        delete(tk);
        pthread_mutex_lock(rd->mutex);
        (*rd->totalbytesread) += bytes_read;
        (*rd->totalreadscount)++;
        pthread_mutex_unlock(rd->mutex);
    }
    free(buffer);
    return 0;
 }




void PrintUsage()
{
    std::cout << endl << endl <<
        "This programme gets from the standard input a sequence of" << endl <<
        " <length> <offset> (one for each line, with <length> less than " << BUFFER_SIZE << "MB in byte)" << endl <<
        " and performs the corresponding read requests towards the given URL or to ALL" << endl <<
        " the URLS contained in the given file." << endl <<
        endl <<
        "Usage: davix-bench [OPTIONS ...] <URL or file>\n" << endl <<
        "  Options:" << endl <<
        "    -h          Display help and usage." << endl <<
        "    -r          Non-vectored read." << endl <<
        "    -vn         Vectored read of vector size n." << endl <<
        "    -tn         Concurrent read of n threads." << endl <<
        "    -d          Debug mode." << endl <<
        "    -s          Silent mode." << endl <<
        "    -w          Write mode, create file which is compatible with the -c option." << endl <<
        "    -i<file>    Read input from file instead of standard input." << endl <<
        "    -c          Verify if the value of the byte at offset i is i%256. Valid only for the non-vectored(r) read mode." << endl << endl;
}
