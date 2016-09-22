#ifndef MONITOR_SETUP_H_
#define MONITOR_SETUP_H_

#include <vector>
#include <string>
#include <algorithm>

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
        auto copyL = lhs.get_edids();
        auto copyR = rhs.get_edids();
        std::sort(copyL.begin(), copyL.end());
        std::sort(copyR.begin(), copyR.end());
        return (copyL == copyR);
    };
    
    inline bool operator!=(const MonitorSetup& lhs, const MonitorSetup& rhs)
    {
        return !operator==(lhs,rhs);
    };
} 

#endif
 
