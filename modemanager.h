#ifndef MODEMANAGER_H_
#define MODEMANAGER_H_

#include "mode.h"
#include "xmanager.h"

#include <string>
#include <vector>
#include <xcb/randr.h>
#include <xcb/xcb.h>


namespace schrandr {
    
    class ModeManager {
    public:
        ModeManager();
        Mode get_current_mode();
        void set_mode(Mode mode);
        void print_screen_info();
        void get_next_event();
    private:    
        Mode current_mode_;
        XManager xmanager_;
    };
} 

#endif 
