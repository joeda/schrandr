#include "screen.h"

namespace schrandr {

    Screen::Screen(xcb_randr_screen_size_t *sizes, int sizes_length)
    {
        for (int i = 0; i < sizes_length; i++) {
            ScreenSize screen;
            screen.width = sizes[i].width;
            screen.height = sizes[i].height;
            screen.mwidth = sizes[i].mwidth;
            screen.mheight = sizes[i].mheight;
            
            sizes_.push_back(screen);
        }
    }
    
    std::string Screen::to_string()const
    {
        std::string res = "Screen\n";
        res += "----------------\n";
        for(auto const& size: sizes_) {
            res += "Size: \n";
            res += "\t----------------\n";
            res += "\tWidth: ";
            res += std::to_string(size.width);
            res += "\n\tHeight: ";
            res += std::to_string(size.height);
            res += "\n\tMWdith: ";
            res += std::to_string(size.mwidth);
            res += "\n\tMHeight: ";
            res += std::to_string(size.mheight);
            res += "\n\t----------------\n";
        }
        res += "----------------\n";
        return res;
    }
}
