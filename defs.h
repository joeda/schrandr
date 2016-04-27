#ifndef DEFS_H_
#define DEFS_H_

namespace schrandr {
    enum schrandr_event_t {
        MODE_EVENT,
        CONNECTION_EVENT,
        OTHER_EVENT
    };
    
    enum x_event_t {
        SCREEN_CHANGE = 89,
        CRTC_CHANGE = 90
    };
}

#endif
