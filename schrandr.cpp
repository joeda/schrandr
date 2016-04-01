#include <iostream>
#include <string>
#include <fstream>

// Required by for routine
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>                                       // flock
#include <memory>                                           // unique_ptr
#include <unistd.h>

#include <stdlib.h>
#include "schrandr.h"

PIDFileHolder::PIDFileHolder(unsigned int pid)
: pid_file_(PID_FILE, std::ios_base::in)
{
    if (!pid_file_.fail()) {
        std::cout << "PID file exists?" << std::endl;
        exit(EXIT_FAILURE);
    }
    pid_file_.close();
    pid_file_.open(PID_FILE, std::ios_base::out | std::ios_base::trunc);
    pid_file_.exceptions(pid_file_.badbit | pid_file_.failbit);
    pid_file_ << pid << std::flush;
}


PIDFileHolder::~PIDFileHolder()
{
    pid_file_.close();
    std::cout << "Now Closing" << std::endl;
    if (unlink(PID_FILE) == -1) {
        exit(EXIT_FAILURE);
    }
    std::cout << "Unlinking worked apparently." << std::endl;
}

bool file_exists(const std::string& name)
{
    struct stat buffer;  
    return (stat (name.c_str(), &buffer) == 0);
}

int main(void)
{
    std::string pid_file = "/tmp/schrandr.pid";
    unsigned int loop_duration = 500000;
    std::unique_ptr<PIDFileHolder> pid_file_holder;
    
    pid_t pID = fork();
    if (pID == 0)                                           // child
    {
        // Code only executed by child process
        std::cout << "I'm the child process!" << std::endl;
        setsid();
        pid_file_holder.reset(new PIDFileHolder(getpid()));
        while (true) {
            std::cout << "Infinite Loop!" << std::endl;
            usleep(loop_duration);
        }
    }
    else if (pID < 0)                                       // failed to fork
    {
        std::cerr << "Failed to fork" << std::endl;
        exit(EXIT_FAILURE);
    // Throw exception
    }
    else                                                    // parent
    {
        // Code only executed by parent process
        std::cout << "I'm the parent process! Goodbye!" << std::endl;  
    }

}
