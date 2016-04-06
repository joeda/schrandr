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
#include <stdarg.h> //va_list
#include <atomic>                                           // aborting
#include <vector>

#include <stdlib.h>
#include <signal.h>                                         // sigaction
#include "schrandr.h"
#include "logging.h"
#include "xmanager.h"

#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>

#include <xcb/xcb.h>

#define OCNE(X) ((XRROutputChangeNotifyEvent*)X)
#define BUFFER_SIZE 128

namespace schrandr {
    
    std::unique_ptr<PIDFileHolder> pid_file_holder;
    std::atomic<int> interruption(0);
    unsigned int loop_duration(500000);
    std::atomic<bool> interactive(false);
    
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
    
    void sig_handler(int signum)
    {
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

int main(int argc, char **argv)
{
    using namespace schrandr;
    
    struct sigaction handler;
    int index;
    int c;
    Logger logger;
    XManager xmanager;
    XRRMonitorInfo *monitor_info;
    XRRScreenResources *screen_resources;
    
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
        
        opterr = 0;
    while ((c = getopt (argc, argv, "i")) != -1) {
        switch (c)
        {
            case 'i':
                interactive = true;
                break;
            case '?':
                if (isprint (optopt))
                    fprintf (stderr, "Unknown option `-%c'.\n", optopt);
                else
                    fprintf (stderr,
                             "Unknown option character `\\x%x'.\n",
                             optopt);
                    return 1;
            default:
                abort ();
        }
    }
    printf("interactive mode? %s\n", interactive ? "true" : "false");
    for (index = optind; index < argc; index++)
        printf ("Non-option argument %s\n", argv[index]);
    
    pid_t pID = 0;
    if (!interactive) {
        pID = fork();
    }
    if (pID == 0)                                           // child
    {
        // Code only executed by child process
        std::cout << "I'm the child process!" << std::endl;
        
        setsid();
        pid_file_holder.reset(new PIDFileHolder(getpid()));
        logger.enable_syslog();
        logger.log("Hello!");
        
        std::vector<std::string> sample_data;
        sample_data.push_back("Bananas");
        sample_data.push_back("Apples");
        logger.log(sample_data);
        logger.log(xmanager.get_monitor_setup());
        
        while (true) {
            std::cout << "Infinite Loop!" << std::endl;
            usleep(loop_duration);
            if (interruption > 0) {
                break;
            }
            logger.log(xmanager.get_X_events());
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
