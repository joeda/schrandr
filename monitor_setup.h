#ifndef MONITOR_SETUP_H_
#define MONITOR_SETUP_H_

#include <vector>
#include <string>

#include "edid.h"

namespace schrandr {  
    
    class MonitorSetup {
    public:
        std::string print_setup();
        void add_edid(Edid e);
        std::vector<Edid> get_edids()const;
        
    private:
        std::vector<Edid> edids_;
    };
    
    inline bool operator==(const MonitorSetup& lhs, const MonitorSetup& rhs)
    {
        bool present;
        if (lhs.get_edids().size() != lhs.get_edids().size()) {
            return false;
        } else {
            for(auto const& lhs_entry: lhs.get_edids()) {
                present = false;
                for(auto const& rhs_entry: rhs.get_edids()) {
                    if (rhs_entry == lhs_entry) {
                        present = true;
                        break;
                    }
                }
                if (!present)
                    return false;
            }
            return true;
        }
    };
    
    inline bool operator!=(const MonitorSetup& lhs, const MonitorSetup& rhs)
    {
        return !operator==(lhs,rhs);
    };
} 

#endif
 
