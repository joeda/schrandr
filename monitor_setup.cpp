#include <iostream>

#include "monitor_setup.h"

namespace schrandr {
    
    MonitorSetup::MonitorSetup() {}
    MonitorSetup::~MonitorSetup() {}
    
    std::vector<std::string> MonitorSetup::print_setup()
    {
        std::vector<std::string> ret;
        for(auto const& monitor: monitors_) {
            std::vector<std::string> moninfo = monitor.to_string();
            std::cout << "This works" << std::endl;
            ret.push_back("Monitor X");
            ret.insert(ret.end(), moninfo.begin(), moninfo.end());
        }
        return ret;
    }
    
    void MonitorSetup::set_monitors(std::vector<Monitor> m)
    {
        monitors_.clear();
        monitors_ = m;
    }
    
    Monitor::Monitor(
        unsigned int xr,
        unsigned int yr,
        unsigned int xos,
        unsigned int yos,
        Edid edid
    ) :
    x_res_(xr),
    y_res_(yr),
    x_on_screen_(xos),
    y_on_screen_(yos),
    edid_(edid),
    active_(true)
    {}
    
    std::vector<std::string> Monitor::to_string()const
    {
        std::vector<std::string> ret;
        ret.push_back("--------");
        std::string edid = "\tEDID: ";
        // this is bugged
        edid += edid_.to_string();
        std::string xres = "\tX-Resolution: ";
        xres += std::to_string(x_res_);
        std::string yres = "\tY-Resolution: ";
        yres += std::to_string(y_res_);
        std::string xpos = "\tX-Position on Screen: ";
        xpos += std::to_string(x_on_screen_);
        std::string ypos = "\tY-Position on Screen: ";
        ypos += std::to_string(y_on_screen_);
        
        ret.push_back(edid);
        ret.push_back(xres);
        ret.push_back(yres);
        ret.push_back(xpos);
        ret.push_back(ypos);
        ret.push_back("--------");
        
        std::cout << "Debug B4" << std::endl;
        
        return ret;
    }
}
