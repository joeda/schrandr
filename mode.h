#ifndef MODE_H_
#define MODE_H_

#include "monitor_setup.h"

#include <string>
#include <vector>
#include <xcb/randr.h>
#include <xcb/xcb.h>


namespace schrandr {
        
    struct Output {
        xcb_randr_output_t output;
        xcb_randr_mode_t mode;
        int x;
        int y;
    };
    
    struct CRTC {
        xcb_randr_crtc_t crtc;
        std::vector<Output> outputs;
    };
    
    class Mode {
    public:
        Mode();
        void add_crtc(CRTC c);
        std::vector<CRTC> get_crtcs();
    
    private:    
        std::vector<CRTC> crtcs_;
    };
} 

#endif
 
 
