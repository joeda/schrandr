#ifndef MONITOR_SETUP_H_
#define MONITOR_SETUP_H_

#include <vector>
#include <string>

#include "edid.h"

namespace schrandr {  

    class Monitor {
    public:
        Monitor(
            unsigned int xr,
            unsigned int yr,
            unsigned int xos,
            unsigned int yos,
            Edid edid
        );
        std::string get_edid()const;
        std::vector<std::string> to_string()const;
    
    private:    
        unsigned int x_res_;
        unsigned int y_res_;
        unsigned int x_on_screen_;
        unsigned int y_on_screen_;
        Edid edid_;
        bool active_;
    };
    
    class MonitorSetup {
    public:
        MonitorSetup();
        ~MonitorSetup();
        std::vector<std::string> print_setup();
        void set_monitors(std::vector<Monitor> m);
        std::vector<Monitor> get_setup()const;
 
    private:
        std::vector<Monitor> monitors_;
    };
            
    inline bool operator==(const Monitor& lhs, const Monitor& rhs)
    {
        if (lhs.get_edid() != rhs.get_edid())
            return false;
        else
            return true;
    };
    
    inline bool operator!=(const Monitor& lhs, const Monitor& rhs)
    {
        return !operator==(lhs,rhs);
    };
    
    inline bool operator==(const MonitorSetup& lhs, const MonitorSetup& rhs)
    {
        bool present;
        if (lhs.get_setup().size() != lhs.get_setup().size()) {
            return false;
        } else {
            for(auto const& lhs_entry: lhs.get_setup()) {
                present = false;
                for(auto const& rhs_entry: rhs.get_setup()) {
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
 
