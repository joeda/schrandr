#include "monitor_setup.h"

namespace schrandr {
    
    MonitorSetup::MonitorSetup() {}
    MonitorSetup::~MonitorSetup() {}
    
    std::vector<std::string> MonitorSetup::print_setup()
    {
        std::vector<std::string> ret;
        
        return ret;
    }
    
    void MonitorSetup::set_monitors(XRandrMonitorInfo m)
    {
        monitors_.clear();
        if (m.n_monitors > 0) {
            for (int i = 0; i < m.n_monitors; i++) {
                Monitor mon_entry;
                mon_entry.x_res = m.minfo[i].width;
                mon_entry.y_res = m.minfo[i].height;
                mon_entry.x_on_screen = m.minfo[i].x;
                mon_entry.y_on_screen = m.minfo[i].y;
                mon_entry.edid = "foo";
                mon_entry.active = true;
                
                monitors_.push_back(mon_entry);
            }
        }
    }
}
