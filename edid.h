#ifndef EDID_H_
#define EDID_H_

#include <string>

namespace schrandr {
    
    enum edid_size_t {
        EDID_v1 = 128,
        EDID_v2 = 256
    };
    
    class Edid {
    public:
        Edid(
            uint8_t *edid,
            size_t array_length
        );
        Edid(std::string edid);
        Edid();
        std::string to_string()const;
        void set_edid(std::string edid);
        void set_edid(uint8_t *edid, size_t array_length);
    private:
        std::string edid_;
        size_t array_length_;
    };
    
    inline bool operator==(const Edid& lhs, const Edid& rhs)
    {
        return lhs.to_string().compare(rhs.to_string());
    };
} 

#endif
 
