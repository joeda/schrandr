#include <iostream>
#include <sstream>
#include <iomanip>

#include "edid.h"

namespace schrandr {
    
    Edid::Edid(uint8_t *edid, size_t array_length):
        edid_array_(edid),
        array_length_(128)
        {
            if (array_length != array_length_)
                std::cout << "Invalid EDID!" << std::endl;
        }
    
    std::string Edid::to_string()const
    {
        std::ostringstream oss;
        for (int i = 0; i < array_length_; i++) {
            oss << std::setw(2) << std::setfill('0') << std::hex << (int) edid_array_[i];
        }
        return oss.str();
    }
} 
 
