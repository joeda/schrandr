#include <iostream>
#include <string>
#include <fstream>
#include <cstring>                                          // memset

// Required by for routine
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>                                       // flock
#include <memory>                                           // unique_ptr
#include <stdarg.h> //va_list
#include <atomic>                                           // aborting
#include <vector>
#include <unistd.h> //fork()

#include <stdlib.h>
#include <signal.h>                                         // sigaction

#include "pid_file_holder.h"
#include "cmdline_args.h"
#include "logging.h"
#include "xmanager.h"
#include "mode.h"
#include "config.h"
#include "defs.h"
#include "util.h"

#include <xcb/xcb.h>


namespace schrandr {
    
    std::unique_ptr<PIDFileHolder> pid_file_holder;
    std::atomic<int> interruption(0);
    
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
    
    auto cmdLineOptions = parseCmdlineArgs(argc, argv);
    if (cmdLineOptions.help) {
        printHelp(std::cout);
        exit(EXIT_SUCCESS);
    }
    if (cmdLineOptions.version) {
        printVersion(std::cout);
        exit(EXIT_SUCCESS);
    }
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
    pid_t pID = 0;
    if (!cmdLineOptions.interactive) {
        pID = fork();
    }
    if (pID == 0)                                           // child
    {
        // Code only executed by child process
        std::cout << "I'm the child process!" << std::endl;
        unsigned int loop_duration(500000);
        std::string mainConfDir;
        auto xdgDir = getenv("XDG_CONFIG_HOME");
        if (xdgDir) {
            mainConfDir = std::string(xdgDir);
        } else {
            auto homeDir = getenv("HOME");
            mainConfDir = std::string(homeDir);
            mainConfDir += "/.config";
        }
        mainConfDir += "/schrandr";
        auto logger = std::make_shared<Logger>();
        XManager xmanager(logger);
        setsid();
        pid_file_holder.reset(new PIDFileHolder(getpid()));
        logger->enable_syslog();
        logger->log("Started schrandr");
        
        Config config(mainConfDir, "config.json", logger);
        bool something_happened;
        Mode prevMode = xmanager.get_mode();
        Mode currentMode = prevMode;
        auto currentMonitorSetup = xmanager.get_monitors();
        auto connectedOutputs = xmanager.getConnectedOutputs();
        config.handleModeChange(currentMonitorSetup, currentMode);
        std::cout << "---MODELIST--- (initially)" << std::endl;
        config.print_modelist();
        std::cout << "---MODELIST END--- (initially)\n" << std::endl;
        
        while (true) {
            usleep(loop_duration);
            if (interruption > 0) {
                //config.write_modes(known_modes);
                break;
            }
            switch(xmanager.check_for_events()) {
            case MODE_EVENT: {
                logger->log("Detected mode change, setting as default for current mode");
                std::cout << "Mode Event" << std::endl;
                prevMode = currentMode;
                currentMode = xmanager.get_mode();
                std::cout << xmanager.get_monitors().print_setup();
                config.handleModeChange(xmanager.get_monitors(), currentMode);
                std::cout << "---MODELIST---" << std::endl;
                config.print_modelist();
                std::cout << "---MODELIST END---\n" << std::endl;
            } break;
            case CONNECTION_EVENT: {
                std::cout << "Connection Event" << std::endl;
                prevMode = currentMode;
                std::cout << "---MODELIST---" << std::endl;
                config.print_modelist();
                std::cout << "---MODELIST END---\n" << std::endl;
                auto monSetup = xmanager.get_monitors();
                std::cout << "MonitorSetup after event:" << std::endl;
                std::cout << monSetup.print_setup();
                auto pastConn = connectedOutputs;
                connectedOutputs = xmanager.getConnectedOutputs();
                auto diff = compareConnectedOutputs(pastConn, connectedOutputs);
                if (config.isMonitorSetupConfigured(monSetup)) {
                    logger->log("Found existing setup, attempting to restore");
                    std::cout << "Found setup in configured setups" << std::endl;
                    Mode toSet = config.getAnyMode(monSetup);
                    std::cout << "Attempting to set mode" << std::endl;
                    xmanager.set_mode(xmanager.get_mode(), toSet);
                }
                else {
                    logger->log("setup not found in configured setups, handling connection events");
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
                logger->log(LOG_ERR, "Internal Event error");
                throw std::runtime_error("Event error");
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
