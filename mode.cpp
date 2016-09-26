#include <iostream>
#include <fstream>

#include "mode.h"
#include "monitor_setup.h"

namespace schrandr {
    
    Mode::Mode()
    {}
    
    void Mode::add_screen(Screen s)
    {
        screens_.push_back(s);
    }
    
    std::vector<Screen> Mode::get_screens() const
    {
        return screens_;
    }
    
    int Mode::eraseCrtc(const xcb_randr_crtc_t &crtc)
    {
        int deleted = 0;
        for (auto screen : screens_) {
            deleted += screen.eraseCrtc(crtc);
        }
        
        return deleted;
    }
    
    Output Mode::getOutputByEdid(const Edid &edid) const
    {
        for (const auto &screen : screens_) {
            for (const auto &crtc : screen.get_crtcs()) {
                for (const auto &output : crtc.outputs) {
                    if (output.edid == edid) {
                        return output;
                    }
                }
            }
        }
        Output empty;
        return empty;
    }
    
    std::vector<Output> Mode::getActiveOutputs() const
    {
        std::vector<Output> res;
        for (const auto &screen : screens_) {
            for (const auto &crtc : screen.get_crtcs()) {
                for (const auto &output : crtc.outputs) {
                    res.push_back(output);
                }
            }
        }
        return res;
    }
    
    CRTC Mode::getCrtcByOutput(const xcb_randr_output_t &output) const
    {
        for (const auto &screen : screens_) {
            for (const auto &crtc : screen.get_crtcs()) {
                for (const auto &op : crtc.outputs) {
                    std::cout << "Output: " << op.output << std::endl;
                    if (op.output == output) {
                        return crtc;
                    }
                }
            }
        }
        CRTC empty;
        return empty;
    }
    
    bool CRTC::hasSameEdids(const CRTC &crtc) const
    {
        std::vector<Edid> self, other;
        for (const auto &op : outputs) {
            self.push_back(op.edid);
        }
        for (const auto &op : crtc.outputs) {
            other.push_back(op.edid);
        }
        std::sort(self.begin(), self.end());
        std::sort(other.begin(), other.end());
        return self == other;
    }
    
    bool CRTC::isEmpty() const
    {
        return outputs.empty();
    }
    
    int Screen::eraseCrtc(const xcb_randr_crtc_t &crtc) 
    {
        int preSize = crtcs_.size();
        crtcs_.erase(std::remove_if(crtcs_.begin(), crtcs_.end(), 
                       [&crtc](CRTC i) { return (i.crtc == crtc); }), crtcs_.end());
        
        return preSize - crtcs_.size();
    }
    
    void Screen::add_crtc(CRTC c)
    {
        crtcs_.push_back(c);
    }
    
    std::vector<CRTC> Screen::get_crtcs()const
    {
        return crtcs_;
    }
    
    bool Screen::operator==(const Screen &lhs)
    {
        return std::tie(width, height, width_mm, height_mm)
            == std::tie(lhs.width, lhs.height, lhs.width_mm, lhs.height_mm);
    }
    
    bool Screen::operator!=(const Screen &lhs)
    {
        return !operator==(lhs);
    }
    
    MonitorSetup Mode::get_monitor_setup() const
    {
        MonitorSetup res;
        for (const auto &screen : screens_) {
            for (const auto &crtc : screen.get_crtcs()) {
                for (const auto &output : crtc.outputs) {
                    if (!output.edid.to_string().empty()) {
                        res.add_edid(output.edid);
                    }
                }
            }
        }
        return res;
    }
}
