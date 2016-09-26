#ifndef MODE_CHANGE_H_
#define MODE_CHANGE_H_

#include <map>

#include "util.h"
#include "mode.h"

namespace schrandr {

class ModeChange {
public:
    ModeChange(
        const Mode &current,
        const Mode &target,
        const std::map<Edid, std::vector<xcb_randr_crtc_t> > &allCrtcsByEdid,
        const std::map<Edid, xcb_randr_output_t> &rawOutputsByEdid);
    std::vector<xcb_randr_crtc_t> getCrtcsToDisable() const;
    std::vector<CRTC> getCrtcChanges() const;
    std::vector<Screen> getScreenChanges() const;
    
private:
    std::vector<xcb_randr_crtc_t> crtcsToDisable_;
    std::vector<CRTC> crtcChanges_;
    std::vector<Screen> screenChanges_;
    
    std::map<xcb_randr_crtc_t, std::pair<CRTC, CRTC> > makeCrtcDiff_(
        const Mode &current,
        const Mode &target,
        const std::map<Edid, std::vector<xcb_randr_crtc_t> > &allCrtcsByEdid);
    std::vector<xcb_randr_crtc_t> getPossibleRawCrtcs_(
        const CRTC &crtc,
        const std::map<Edid, std::vector<xcb_randr_crtc_t> > &allCrtcsByEdid);
};

} //schrandr

#endif
