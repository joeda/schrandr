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
        
    private:    
        Mode current_mode_;
        XManager xmanager_;
        void set_mode_(Mode mode);
    };
} 

#endif 
