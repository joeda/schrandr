#ifndef PID_FILE_HOLDER_H_
#define PID_FILE_HOLDER_H_

#include <unistd.h> //unlink

#include "pid_file_holder.h"

namespace schrandr {
    
PIDFileHolder::PIDFileHolder(
    unsigned int pid,
    const std::string pid_file /*= std::string("/tmp/schrandr.pid")*/)
: pid_file_(pid_file, std::ios_base::in),
  pid_file_path_(pid_file)
{
    if (!pid_file_.fail()) {
        std::cout << "PID file exists?" << std::endl;
        exit(EXIT_FAILURE);
    }
    pid_file_.close();
    pid_file_.open(pid_file_path_, std::ios_base::out | std::ios_base::trunc);
    pid_file_.exceptions(pid_file_.badbit | pid_file_.failbit);
    pid_file_ << pid << std::flush;
}


PIDFileHolder::~PIDFileHolder()
{
    pid_file_.close();
    std::cout << "Now Closing" << std::endl;
    if (unlink(pid_file_path_.c_str()) == -1) {
        exit(EXIT_FAILURE);
    }
    std::cout << "Unlinking worked apparently." << std::endl;
}

}

#endif
