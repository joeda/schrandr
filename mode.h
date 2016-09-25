#ifndef MODE_H_
#define MODE_H_

#include "monitor_setup.h"
#include "edid.h"

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
        Edid edid;
        std::string name;
    };
    
    struct CRTC {
        xcb_randr_crtc_t crtc;
        std::vector<Output> outputs;
    };
    
    class Screen {
    public:
        int width;
        int height;
        int width_mm;
        int height_mm;
        std::vector<CRTC> crtcs_;
        void add_crtc(CRTC c);
        int eraseCrtc(const xcb_randr_crtc_t &crtc);
        std::vector<CRTC> get_crtcs()const;
    };
    
    class Mode {
    public:
        Mode();
        void add_screen(Screen s);
        std::vector<Screen> get_screens() const;
        MonitorSetup get_monitor_setup() const;
        void set_monitor_setup(MonitorSetup s);
        int eraseCrtc(const xcb_randr_crtc_t &crtc);
        Output getOutputByEdid(const Edid &edid) const;
        std::vector<Output> getActiveOutputs() const;
        CRTC getCrtcByOutput(const xcb_randr_output_t &output) const;

    
    private:    
        std::vector<Screen> screens_;
    };
} 

#endif
 
 
