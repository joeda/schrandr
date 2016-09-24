#include <iostream>
#include <fstream>

#include "mode.h"
#include "monitor_setup.h"

namespace schrandr {
    
    Mode::Mode()
    {}
    
    void Mode::add_screen(Screen s)
    {
        screens_.push_back(s);
    }
    
    std::vector<Screen> Mode::get_screens()
    {
        return screens_;
    }
    
    int Mode::eraseCrtc(const xcb_randr_crtc_t &crtc)
    {
        int deleted = 0;
        for (auto screen : screens_) {
            deleted += screen.eraseCrtc(crtc);
        }
        
        return deleted;
    }
    
    int Screen::eraseCrtc(const xcb_randr_crtc_t &crtc)
    {
        int preSize = crtcs_.size();
        crtcs_.erase(std::remove_if(crtcs_.begin(), crtcs_.end(), 
                       [&crtc](CRTC i) { return (i.crtc == crtc); }), crtcs_.end());
        
        return preSize - crtcs_.size();
    }
    
    void Screen::add_crtc(CRTC c)
    {
        crtcs_.push_back(c);
    }
    
    std::vector<CRTC> Screen::get_crtcs()const
    {
        return crtcs_;
    }
    
    MonitorSetup Mode::get_monitor_setup()
    {
        MonitorSetup res;
        for (const auto &screen : screens_) {
            for (const auto &crtc : screen.get_crtcs()) {
                for (const auto &output : crtc.outputs) {
                    if (!output.edid.to_string().empty()) {
                        res.add_edid(output.edid);
                    }
                }
            }
        }
        return res;
    }
}
