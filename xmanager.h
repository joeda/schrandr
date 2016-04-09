#ifndef XINTERFACE_H_
#define XINTERFACE_H_

#include <iostream>
#include <memory>
#include <vector>

#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>

#include <xcb/xcb.h>

#include "monitor_setup.h"

namespace schrandr {
    
    class XManager {
    public:
        XManager();
        ~XManager();
        std::vector<std::string> get_X_events();
        std::vector<std::string> get_monitor_info();
        std::vector<std::string> get_monitor_setup();
        XRandrMonitorInfo get_monitors();
        std::string get_edid();

    private:
        xcb_connection_t *xcb_connection_;
        Display *dpy_;
        Window root_;
        int screen_;
        XEvent ev_;
        XPointer dummy_;
        std::vector<std::string> con_actions;
        
        int (*predicate_)(Display*, XEvent*, XPointer);
        
        int error_handler_(void);
        bool has_randr_15_(Display *dpy_);
        XRRMonitorInfo* get_monitors_(Display *dpy, Window root);
        std::vector<std::string> monitor_info_to_string_(XRRMonitorInfo *monitor_info);
    };
} 

#endif 
