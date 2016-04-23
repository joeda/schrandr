#include <iostream>
#include <sstream>
#include <iomanip>

#include "edid.h"

namespace schrandr {
    
    Edid::Edid(uint8_t *edid, size_t array_length):
        array_length_(128)
        {
            if (array_length != array_length_) {
                std::cout << "Invalid EDID!" << std::endl;
                exit(1);
            }
            std::ostringstream oss;
            for (int i = 0; i < array_length_; i++) {
                oss << std::setw(2) << std::setfill('0') << std::hex << (int) edid[i];
            }
            edid_ = oss.str();
        }
    
    Edid::Edid(std::string edid):
        edid_(edid),
        array_length_(128)
        {}
        
    Edid::Edid():
        array_length_(128)
        {}
    
    std::string Edid::to_string()const
    {
        return edid_;
    }
    
    void Edid::set_edid(std::string edid)
    {
        edid_ = edid;
    }
    
     void Edid::set_edid(uint8_t *edid, size_t array_length)
    {
        if (array_length != array_length_) {
            std::cout << "Invalid EDID!" << std::endl;
            exit(1);
        }
        std::ostringstream oss;
        for (int i = 0; i < array_length_; i++) {
            oss << std::setw(2) << std::setfill('0') << std::hex << (int) edid[i];
        }
        edid_ = oss.str();
    }
} 
 
