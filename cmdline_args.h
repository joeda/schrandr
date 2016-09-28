#ifndef CMDLINE_ARGS_H_
#define CMDLINE_ARGS_H_

#include <string>
#include <ostream>

namespace schrandr {

struct CmdLineOptions {
    bool interactive = false;
    bool help = false;
    bool version = false;
    std::string confDir;
};

CmdLineOptions parseCmdlineArgs(int argc, char **argv);

void printHelp(std::ostream &out);
void printVersion(std::ostream &out);
}

#endif
