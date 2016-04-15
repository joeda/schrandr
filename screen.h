#ifndef SCREEN_H_
#define SCREEN_H_

#include <string>
#include <vector>
#include <xcb/randr.h>
#include <xcb/xcb.h>


namespace schrandr {
    
    struct ScreenSize {
        unsigned int width;
        unsigned int height;
        unsigned int mwidth;
        unsigned int mheight;
    };

    class Screen {
    public:
        Screen(
            xcb_randr_screen_size_t *sizes,
            int sizes_length
        );
        std::string to_string()const;
    
    private:    
        std::vector<ScreenSize> sizes_;
    };
}

#endif 
