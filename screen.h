#ifndef SCREEN_H_
#define SCREEN_H_

#include <string>

namespace schrandr {  

    class Screen {
    public:
        Screen(
            unsigned int xr,
            unsigned int yr,
            unsigned int mx,
            unsigned int my
        );
        std::string to_string()const;
    
    private:    
        unsigned int width_;
        unsigned int height_;
        unsigned int mwidth_;
        unsigned int mheight_;
    };
}

#endif 
