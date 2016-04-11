#ifndef XINTERFACE_H_
#define XINTERFACE_H_

#include <iostream>
#include <memory>
#include <vector>

//#include <X11/Xlib.h>
//#include <X11/extensions/Xrandr.h>

#include <xcb/xcb.h>

#include "monitor_setup.h"

namespace schrandr {
    
    class XManager {
    public:
        XManager();
        ~XManager();
        std::vector<Monitor> get_monitors();

    private:
        xcb_connection_t *xcb_connection_;
                
        int error_handler_(void);
        //bool has_randr_15_(Display *dpy_);
    };
} 

#endif 
