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
#include "mode.h"
#include "config.h"
#include "defs.h"
#include "util.h"

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
        std::string err = std::strerror(errno);
	try {
            std::rethrow_exception(std::current_exception());
	} catch (const std::exception &e) {
            std::cerr << "Unhandled exception: " << typeid(e).name() << ": " << e.what() << "." << std::endl <<
                "errno = " << err << "." << std::endl;
        }
    }
    
    std::pair<std::vector<xcb_randr_output_t>, std::vector<xcb_randr_output_t> >
    compareConnectedOutputs(const std::vector<xcb_randr_output_t> &pre,
        const std::vector<xcb_randr_output_t> &current)
    {
        std::vector<xcb_randr_output_t> disconnected;
        std::vector<xcb_randr_output_t> connected;
        for (const auto &conn : pre) {
            if (std::find(current.begin(), current.end(), conn) == current.end()) {
                disconnected.push_back(conn);
            }
        }
        for (const auto &conn : current) {
            if (std::find(pre.begin(), pre.end(), conn) == pre.end()) {
                connected.push_back(conn);
            }
        }
        
        return std::make_pair(connected, disconnected);
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
        
        Config config;
        bool something_happened;
        Mode prevMode = xmanager.get_mode();
        Mode currentMode = prevMode;
        ModeList known_modes;
        auto currentMonitorSetup = xmanager.get_monitors();

        known_modes.addNamedMode(currentMonitorSetup, prevMode, "");
        //config.print_modelist(known_modes);
        auto connectedOutputs = xmanager.getConnectedOutputs();
        /* for (const auto &output : connectedOutputs) {
            std::cout << "Available modes for output " << std::to_string(output)
                      << ":";
            for (const auto &mode : xmanager.getAvailableModesFromOutput(output)) {
                std::cout << " " << std::to_string(mode);
            }
            std::cout << std::endl;
        }
        std::cout << "Outputs active post-event: " << std::endl;
        for (const auto &conn : xmanager.getActiveOutputs()) {
            std::cout << "Active: " << std::to_string(conn)
                    << std::endl;
        } */
        // Apparently, the first mode has always the highest resolution
        /* 
        for (const auto &mi : xmanager.getModeInfos()) {
            std::cout << "Begin Mode info:" << std::endl;
            std::cout << "  id: " << mi.id << "\n  w: " << mi.width
                      << "\n  h: " << mi.height << "\n  mode_flags: "
                      << mi.mode_flags << std::endl;
        }
        */
        std::cout << "---MODELIST--- (initially)" << std::endl;
        config.print_modelist(known_modes);
        std::cout << "---MODELIST END--- (initially)\n" << std::endl;
        
        while (true) {
            std::cout << "Infinite Loop!" << std::endl;
            usleep(loop_duration);
            if (interruption > 0) {
                config.write_modes(known_modes);
                break;
            }
            switch(xmanager.check_for_events()) {
            case MODE_EVENT: {
                std::cout << "Mode Event" << std::endl;
                prevMode = currentMode;
                currentMode = xmanager.get_mode();
                std::cout << xmanager.get_monitors().print_setup();
                known_modes.setDefaultMode(
                    xmanager.get_monitors(), currentMode);
                std::cout << "---MODELIST---" << std::endl;
                config.print_modelist(known_modes);
                std::cout << "---MODELIST END---\n" << std::endl;
            } break;
            case CONNECTION_EVENT: {
                std::cout << "Connection Event" << std::endl;
                prevMode = currentMode;
                std::cout << "---MODELIST---" << std::endl;
                config.print_modelist(known_modes);
                std::cout << "---MODELIST END---\n" << std::endl;
                auto monSetup = xmanager.get_monitors();
                std::cout << "MonitorSetup after event:" << std::endl;
                std::cout << monSetup.print_setup();
                auto pastConn = connectedOutputs;
                connectedOutputs = xmanager.getConnectedOutputs();
                auto diff = compareConnectedOutputs(pastConn, connectedOutputs);
                if (known_modes.isMonitorSetupConfigured(monSetup)) {
                    std::cout << "Found setup in configured setups" << std::endl;
                    Mode toSet = known_modes.getAnyMode(monSetup);
                    std::cout << "Attempting to set mode" << std::endl;
                    xmanager.set_mode(xmanager.get_mode(), toSet);
                }
                else {
                    std::cout << "setup not found in configured setups" << std::endl;
                    for (const auto &conn : diff.second) {
                        std::cout << "disconnected: " << std::to_string(conn)
                                << std::endl;
                        xmanager.disableOutput(conn, prevMode);
                    }
                    /* auto m = xmanager.get_mode();
                    std::cout << "POST (DIS)CONNECTION----" << std::endl;
                    //config.print_mode(m);
                    std::cout << "POST (DIS)CONNECTION----" << std::endl; */
                    if (xmanager.getActiveOutputs().empty())
                    {
                        std::cout << "Apparently no output active" << std::endl;
                        xmanager.activateAnyOutput();
                    }
                }
                std::cout << "Outputs active post-event: " << std::endl;
                for (const auto &conn : xmanager.getActiveOutputs()) {
                    std::cout << "Active: " << std::to_string(conn)
                            << std::endl;
                }
                std::cout << "Outputs connected post-event: ";
                for (const auto &c : connectedOutputs) {
                    std::cout << " " << std::to_string(c);
                }
                std::cout << std::endl;
                currentMode = xmanager.get_mode();
            } break;
            case OTHER_EVENT:
                break;
            default:
                std::cout << "Default. WTF!?" << std::endl;
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
