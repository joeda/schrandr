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
    char *con_actions[] = { "connected", "disconnected", "unknown" };
    
    
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
    
    static void xerror(const char *format, ...)
    {
        va_list args;
        va_start(args, format);
        vfprintf(stderr, format, args);
        va_end(args);
        exit(EXIT_FAILURE);
    }
    
    static int error_handler(void)
    {
        exit(EXIT_FAILURE);
    }
    
    int predicate_event(Display *display, XEvent *ev, XPointer arg) {
        return true;
    }
    
    std::vector<std::string> get_monitor_setup(xcb_connection_t *xcb_conn) {
        std::vector<std::string> monitors;
        const xcb_setup_t *setup;
        xcb_screen_iterator_t iter;
        xcb_screen_t *screen;
        
        setup = xcb_get_setup(xcb_conn);
        iter = xcb_setup_roots_iterator(setup);
        screen = iter.data;
        std::string screen_res = "Screen X Res: ";
        screen_res += std::to_string(screen->width_in_pixels);
        screen_res += " Screen Y Res: ";
        screen_res += std::to_string(screen->height_in_pixels);
        monitors.push_back(screen_res);
        
        return monitors;
    }
    
    bool has_randr_15(Display *dpy)
    {
        int major, minor;
        int event_base, error_base;
        if (!XRRQueryExtension (dpy, &event_base, &error_base) ||
        !XRRQueryVersion (dpy, &major, &minor))
        {
            fprintf (stderr, "RandR extension missing\n");
            exit (EXIT_FAILURE);
        }
        if (major > 1 || (major == 1 && minor >= 5))
            return true;
        else
            return false;
    }
    
    XRRMonitorInfo* get_monitors(Display *dpy, Window root)
    {
        XRRMonitorInfo *m;
        int n;
        m = XRRGetMonitors(dpy, root, false, &n);
        if (n == -1) {
            fprintf(stderr, "get monitors failed\n");
            exit(EXIT_FAILURE);
        }
        return m;
    }
    
    std::vector<std::string> monitor_info_to_string
    (XRRMonitorInfo *monitor_info)
    {
        std::vector<std::string> info;
        info.push_back("---- Monitor Info ----");
        
        std::string x = "x: ";
        x += std::to_string(monitor_info->x);
        info.push_back(x);
        std::string y = "y: ";
        y += std::to_string(monitor_info->y);
        info.push_back(y);
        std::string noutput = "noutput: ";
        noutput += std::to_string(monitor_info->noutput);
        info.push_back(noutput);
        std::string width = "width: ";
        width += std::to_string(monitor_info->width);
        info.push_back(width);
        std::string height = "height: ";
        height += std::to_string(monitor_info->height);
        info.push_back(height);
        
        return info;
    }
    
}

int main(int argc, char **argv)
{
    using namespace schrandr;
    
    struct sigaction handler;
    int index;
    int c;
    XEvent ev;
    Display *dpy;
    char buf[BUFFER_SIZE];
    XPointer dummy;
    Logger logger;
    char log_buf[BUFFER_SIZE];
    xcb_connection_t *xcb_connection;
    static Window root;
    int screen;
    XRRMonitorInfo *monitor_info;
    
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
        xcb_connection = xcb_connect (NULL, NULL);
        
        std::vector<std::string> sample_data;
        sample_data.push_back("Bananas");
        sample_data.push_back("Apples");
        logger.log(sample_data);
        logger.log(get_monitor_setup(xcb_connection));
        
        if ((dpy = XOpenDisplay(NULL)) == NULL)
            xerror("Cannot open display\n");
        std::cout << "Hello #0" << std::endl;
        XRRSelectInput(dpy, DefaultRootWindow(dpy), RROutputChangeNotifyMask);
        std::cout << "Hello #1" << std::endl;
        XSync(dpy, False);
        std::cout << "Hello #2" << std::endl;
        XSetIOErrorHandler((XIOErrorHandler) error_handler);
        screen = DefaultScreen (dpy);
        root = RootWindow (dpy, screen);
        if (has_randr_15(dpy)) {
            monitor_info = get_monitors(dpy, root);
            std::vector<std::string> minfo =
                monitor_info_to_string(monitor_info);
            logger.log(minfo);
        }
        
        int (*predicate)(Display*, XEvent*, XPointer);
        predicate = &predicate_event;
        
        while (true) {
            std::cout << "Infinite Loop!" << std::endl;
            usleep(loop_duration);
            if (interruption > 0) {
                break;
            }
            if (XCheckIfEvent(dpy, &ev, predicate, dummy)) {
                std::cout << "Hello #3" << std::endl;
                XRRScreenResources *resources = XRRGetScreenResources(OCNE(&ev)->display,
                                                                      OCNE(&ev)->window);
                std::cout << "Hello #4" << std::endl;
                if (resources == NULL) {
                    fprintf(stderr, "Could not get screen resources\n");
                    continue;
                }
                std::cout << "Hello #5" << std::endl;
                XRROutputInfo *info = XRRGetOutputInfo(OCNE(&ev)->display, resources,
                                                       OCNE(&ev)->output);
                if (info == NULL) {
                    XRRFreeScreenResources(resources);
                    fprintf(stderr, "Could not get output info\n");
                    continue;
                }
                
                snprintf(buf, BUFFER_SIZE, "%s %s", info->name,
                         con_actions[info->connection]);
                printf("Event: %s %s\n", info->name,
                       con_actions[info->connection]);
                snprintf(log_buf, BUFFER_SIZE, "Event: %s %s\n", info->name, 
                           con_actions[info->connection]);
                logger.log(log_buf);
                printf("Time: %lu\n", info->timestamp);
                snprintf(log_buf, BUFFER_SIZE, "Time: %lu\n", info->timestamp);
                logger.log(log_buf);
                if (info->crtc == 0) {
                    printf("Size: %lumm x %lumm\n", info->mm_width, info->mm_height);
                }
                else {
                    printf("CRTC: %lu\n", info->crtc);
                    XRRCrtcInfo *crtc = XRRGetCrtcInfo(dpy, resources, info->crtc);
                    if (crtc != NULL) {
                        printf("Size: %dx%d\n", crtc->width, crtc->height);
                        XRRFreeCrtcInfo(crtc);
                    }
                }
                XRRFreeScreenResources(resources);
                XRRFreeOutputInfo(info);
            } else {
                std::cout << "No XNextEvent" << std::endl;
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
