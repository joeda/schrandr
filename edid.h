#ifndef EDID_H_
#define EDID_H_

#include <string>

namespace schrandr {
    
    class Edid {
    public:
        Edid(
            uint8_t *edid,
            size_t array_length
        );
        std::string to_string()const;
    private:
        uint8_t *edid_array_;
        size_t array_length_;
    };
} 

#endif
 
