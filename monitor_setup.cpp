#include <iostream> 

#include "monitor_setup.h"

namespace schrandr {
    
    std::string MonitorSetup::print_setup()
    {
        std::string res("This Setup has ");
        res.append(std::to_string(edids_.size()));
        res. append(" connected devices.\n");
        res.append("------------\n");
        for(auto const& edid: edids_) {
            res.append("\t");
            res.append(edid.to_string());
            res.append("\n");
        }
        res.append("------------\n");
        
        return res;
    }
    
    std::vector<Edid> MonitorSetup::get_edids()const
    {
        return edids_;
    }
    
    void MonitorSetup::add_edid(Edid e)
    {
        edids_.push_back(e);
    }
}
