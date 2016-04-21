#ifndef XINTERFACE_H_
#define XINTERFACE_H_

#include <iostream>
#include <memory>
#include <vector>

//#include <X11/Xlib.h>
//#include <X11/extensions/Xrandr.h>

#include <xcb/xcb.h>
#include <xcb/randr.h>

#include "monitor_setup.h"
#include "mode.h"

namespace schrandr {
    
    class XManager {
    public:
        XManager();
        ~XManager();
        std::vector<Monitor> get_monitors();
        Mode get_mode();
        void set_mode(Mode m);
        void print_screen_info();
        void get_next_event();

    private:
        xcb_connection_t *xcb_connection_;
        xcb_window_t window_dummy_;
        xcb_randr_output_t *outputs_;
        int n_outputs_;
        xcb_screen_t *screens_;
        int n_screens_;
        xcb_generic_event_t *ev_;
        
        void get_screens_raw_();
        void get_outputs_raw_();
        void make_window_dummy_();
                
        int error_handler_(void);
        //bool has_randr_15_(Display *dpy_);
    };
} 

#endif 
