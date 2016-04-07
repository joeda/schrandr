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
    
    class Monitor {
    public:
        Monitor(
            unsigned int xr,
            unsigned int yr,
            unsigned int xos,
            unsigned int yos
        );
        std::vector<std::string> to_string()const;
    
    private:    
        unsigned int x_res_;
        unsigned int y_res_;
        unsigned int x_on_screen_;
        unsigned int y_on_screen_;
        std::string edid_;
        bool active_;
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
 
