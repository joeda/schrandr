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
    
    void Screen::add_crtc(CRTC c)
    {
        crtcs_.push_back(c);
    }
    
    std::vector<CRTC> Screen::get_crtcs()const
    {
        return crtcs_;
    }
}

