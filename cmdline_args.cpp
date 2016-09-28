#include <unistd.h> //getopt

#include "cmdline_args.h"

namespace schrandr {

CmdLineOptions parseCmdlineArgs(int argc, char **argv) {
    CmdLineOptions options;
    char *confDir = nullptr;
    bool hasConfDir = false;

    int index;
    int c;
    while ((c = getopt (argc, argv, "ivhc:")) != -1) {
        switch (c)
        {
        case 'i':
            options.interactive = true;
            break;
        case 'v':
            options.version = true;
            break;
        case 'h':
            options.help = true;
        case 'c': {
            confDir = optarg;
            hasConfDir = true;
            }
        case '?':
            if (isprint (optopt))
                fprintf (stderr, "Unknown option `-%c'.\n", optopt);
            else
                fprintf (stderr,
                            "Unknown option character `\\x%x'.\n",
                            optopt);
        default:
            abort ();
        }
    }
    for (index = optind; index < argc; index++)
        printf ("Non-option argument %s\n", argv[index]);
    if (hasConfDir) {
        options.confDir = std::string(confDir);
    }
    
    return options;
}

void printHelp(std::ostream &out) {
    out << "Possible flags:"
        << "\t-i\t\trun interactively, do not become a daemon\n"
        << "\t-v\t\tprint version info\n"
        << "\t-c DIR\t\tuse custom configuration directory"
        << "\t-h\t\tprint this help" << std::endl;
}

void printVersion(std::ostream &out) {
    out << "Schrandr 0.0.0" << std::endl;
}
}
