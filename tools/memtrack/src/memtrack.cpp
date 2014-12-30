/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#pragma once

#include "memlog.h"
#include "ioaccess.h"
#include "getopt.h"
#include "memtracker.h"

static void print_usage() {
    printf("mem_track: ");
    printf(
        "Usage: mem_track [options] filepath1 \n"
        "Available options are:\n"
        "\tlist-markers           - list all heap markers in file\n"
        "\toutput, o              - output to file. Without this option stdout is used.\n"
        );
}


enum class Option : int { // ensure these match options above
    optOutputToFile = 0,
    optListMarkers,
    optListAllLeaks,
    optMarker,
    optLeakCheck,
};

const char options[] = { "o:m:l" };
static struct option long_options[] = {
    { "list-markers", no_argument, 0, 'z' },
    { "output", required_argument, 0, 'o' },
    { "list-all-leaks", no_argument, 0, 'a' },
    { "marker", required_argument, 0, 'm' },
    { "leak-check", required_argument, 0, 'l' },
    { 0, 0, 0, 0 }
};

struct Options {
    Options()
        : listMarkers(false) 
        , listAllLeaks(false) {
    }
    std::string inputFile;
    std::string outputFile_;
    std::vector<std::string> markers;
    bool        listMarkers : 1;
    bool        listAllLeaks : 1;
    bool        leakCheck : 1;
};

bool getOptions(int argc, char **argv, Options* out_opts) {
    int c;
    for (;;) {
        /* getopt_long stores the option index here. */
        Option opt;
        int option_index = 0;

        c = gop_getopt_long(argc, argv, options, long_options, &option_index);

        /* Detect the end of the options. */
        if (c == -1)
            break;

        switch (c) {
        case 0: opt = (Option)option_index; break;
        case 'z': opt = Option::optListMarkers; break;
        case 'o': opt = Option::optOutputToFile; break;
        case 'a': opt = Option::optListAllLeaks; break;
        case 'm': opt = Option::optMarker; break;
        case 'l': opt = Option::optLeakCheck; break;
        case '?':
            /* getopt_long already printed an error message. */
        default:
            return false;
        }

        switch (opt)
        {
        case Option::optListMarkers: out_opts->listMarkers = true; break;
        case Option::optOutputToFile: out_opts->outputFile_ = optarg; break;
        case Option::optListAllLeaks: out_opts->listAllLeaks = true; break;
        case Option::optMarker: out_opts->markers.push_back(optarg); break;
        case Option::optLeakCheck: out_opts->leakCheck = true; break;
        default:
            break;
        }
    }

    /* For any remaining command line arguments (not options) take the first, ignore the rest */
    if (optind < argc) {
        out_opts->inputFile = argv[optind];
        return true;
    }

    return false;
}

int main(int argc, char* argv[]) {
    Options options;
    if (!getOptions(argc, argv, &options)) {
        print_usage();
    }
    FILE* perr = stderr;
    FILE* pout = stdout;

    MemLog* mem_log;
    IODevice io;
    if (parseMemLog(options.inputFile.c_str(), &mem_log, &io) != 0) {
        fprintf(perr, "Error reading memory trace %s\n", options.inputFile.c_str());
        return 1;
    }

    if (options.listMarkers) {
        mem_log->listAllMarkers(pout);
    }
    if (options.listAllLeaks) {
        mem_log->writeAllLeaks(pout, 0, ~0ull);
    } else if(options.leakCheck) {
        mt_uint64 f,l;
        if (!mem_log->getMarkers(pout, options.markers, &f, &l)) {
            fprintf(perr, "Error : couldn't find at least 2 markers in trace\n");
            return 1;
        }
        mem_log->writeAllLeaks(pout, f, l);
    }

    return 0;
}