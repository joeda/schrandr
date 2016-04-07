#include "monitor_setup.h"

namespace schrandr {
    
    MonitorSetup::MonitorSetup() {}
    MonitorSetup::~MonitorSetup() {}
    
    std::vector<std::string> MonitorSetup::print_setup()
    {
        std::vector<std::string> ret;
        for(auto const& monitor: monitors_) {
            std::vector<std::string> moninfo = monitor.to_string();
            ret.push_back("Monitor X");
            ret.insert(ret.end(), moninfo.begin(), moninfo.end());
        }
        
        return ret;
    }
    
    void MonitorSetup::set_monitors(XRandrMonitorInfo m)
    {
        monitors_.clear();
        if (m.n_monitors > 0) {
            for (int i = 0; i < m.n_monitors; i++) {
                Monitor mon_entry(
                    m.minfo[i].width,
                    m.minfo[i].height,
                    m.minfo[i].x,
                    m.minfo[i].y
                        );
                monitors_.push_back(mon_entry);
            }
        }
    }
    
    Monitor::Monitor(
        unsigned int xr,
        unsigned int yr,
        unsigned int xos,
        unsigned int yos
    ) :
    x_res_(xr),
    y_res_(yr),
    x_on_screen_(xos),
    y_on_screen_(yos),
    edid_("foo_EDID"),
    active_(true)
    {}
    
    std::vector<std::string> Monitor::to_string()const
    {
        std::vector<std::string> ret;
        ret.push_back("--------");
        std::string xres = "\tX-Resolution: ";
        xres += std::to_string(x_res_);
        std::string yres = "\tY-Resolution: ";
        yres += std::to_string(y_res_);
        std::string xpos = "\tX-Position on Screen: ";
        xpos += std::to_string(x_on_screen_);
        std::string ypos = "\tY-Position on Screen: ";
        ypos += std::to_string(y_on_screen_);
        
        ret.push_back(xres);
        ret.push_back(yres);
        ret.push_back(xpos);
        ret.push_back(ypos);
        ret.push_back("--------");
        
        return ret;
    }
}
