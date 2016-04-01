#include <iostream>
#include <string>
#include <fstream>
#include <cstring>                                          // memset

// Required by for routine
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>                                       // flock
#include <memory>                                           // unique_ptr
#include <unistd.h>
#include <atomic>                                           // aborting

#include <stdlib.h>
#include <signal.h>                                         // sigaction
#include "schrandr.h"

namespace schrandr {

std::unique_ptr<PIDFileHolder> pid_file_holder;
std::atomic<int> interruption(0);
unsigned int loop_duration(500000);

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

void sig_handler(int signum) {
    switch(signum) {
        case SIGHUP:
        case SIGINT:
        case SIGTERM:
            interruption = signum;
            break;
        case SIGUSR1:
            break;
        case SIGSEGV:
        // Let's hope memory isn't too fucked up to get through with this ;)
            break;
        case SIGUSR2:
            interruption = signum;
            break;
    }
}


void handle_uncaught()
{
    std::cout << "Panic!" << std::endl;
}
}

int main(void)
{
    using namespace schrandr;

    struct sigaction handler;

    std::set_terminate(handle_uncaught);

    memset(&handler, 0, sizeof(handler));
    handler.sa_handler = sig_handler;

    // Install signal handler only after FanControl object has been created
    // since it is used by the handler.
    if (sigaction(SIGHUP, &handler, NULL)
        || sigaction(SIGINT, &handler, NULL)
        || sigaction(SIGTERM, &handler, NULL)
        || sigaction(SIGUSR1, &handler, NULL)
        || sigaction(SIGUSR2, &handler, NULL)
        || sigaction(SIGSEGV, &handler, NULL)) {
            return 1;
    }

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
            if (interruption > 0) {
                break;
              }
          }
        return EXIT_SUCCESS;
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
