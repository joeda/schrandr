#ifndef MONITOR_SETUP_H_
#define MONITOR_SETUP_H_

#include <vector>
#include <string>

#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>

namespace schrandr {
    
    struct XRandrMonitorInfo
    {
        XRRMonitorInfo* minfo;
        int n_monitors;
    };
    
    struct Monitor
    {
        unsigned int x_res;
        unsigned int y_res;
        unsigned int x_on_screen;
        unsigned int y_on_screen;
        std::string edid;
        bool active;
    };
    
    class MonitorSetup {
    public:
        MonitorSetup();
        ~MonitorSetup();
        std::vector<std::string> print_setup();
        void set_monitors(XRandrMonitorInfo m);
 
    private:
        std::vector<Monitor> monitors_;
    };
} 

#endif
 
