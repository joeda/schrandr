#include "screen.h"

namespace schrandr {

    Screen::Screen(
        unsigned int xr,
        unsigned int yr,
        unsigned int mx,
        unsigned int my
    ):
        width_(xr),
        height_(yr),
        mwidth_(mx),
        mheight_(my)
    {}
    
    std::string Screen::to_string()const
    {
        std::string res = "Screen\n";
        res += "----------------\n";
        res += "\tWidth: ";
        res += std::to_string(width_);
        res += "\n\tHeight: ";
        res += std::to_string(height_);
        res += "\n\tMWdith: ";
        res += std::to_string(mwidth_);
        res += "\n\tMHeight: ";
        res += std::to_string(mheight_);
        res += "\n----------------";
        
        return res;
    }
}
