#ifndef XINTERFACE_H_
#define XINTERFACE_H_

#include <iostream>
#include <memory>
#include <vector>
#include <map>

#include <xcb/xcb.h>
#include <xcb/randr.h>

#include "mode_change.h"
#include "monitor_setup.h"
#include "mode.h"
#include "defs.h"

namespace schrandr {
    
    class XManager {
    public:
        XManager();
        ~XManager();
        MonitorSetup get_monitors();
        Mode get_mode();
        void set_mode(const Mode &current, const Mode &target);
        void print_screen_info();
        schrandr_event_t check_for_events();
        std::vector<xcb_randr_output_t> getConnectedOutputs();
        std::vector<xcb_randr_output_t> getActiveOutputs();
        bool disableOutput(
            const xcb_randr_output_t &output, const Mode &lastConnected);
        bool disableCrtc(const CRTC &crtc);
        bool disableCrtc(const xcb_randr_crtc_t &crtc);
        bool activateAnyOutput();
        std::vector<xcb_randr_mode_t> 
            getAvailableModesFromOutput(const xcb_randr_output_t &output);
        std::vector<xcb_randr_mode_info_t> getModeInfos();

    private:
        xcb_connection_t *xcb_connection_;
        xcb_window_t window_dummy_;
        xcb_randr_output_t *outputs_;
        int n_outputs_;
        xcb_screen_t *screens_;
        int n_screens_;
        int screen_;
        
        void get_screens_raw_();
        void get_outputs_raw_();
        bool get_outputs_raw_(xcb_randr_output_t **outputs, int *n_outputs);
        bool get_crtcs_raw_(xcb_randr_crtc_t **crtcs, int *nCrtcs);
        std::map<Edid, xcb_randr_output_t> getOuputsByEdid_();
        std::map<Edid, std::vector<xcb_randr_crtc_t> > getCrtcsByEdid_();
        
        void make_window_dummy_();
        void refreshAll_();
        Edid get_edid_(const xcb_randr_output_t &output);
        std::vector<std::string> getAvailableAtoms(
            const xcb_randr_output_t &output);
        bool getCrtcByOutput_(
            const xcb_randr_output_t &output, xcb_randr_crtc_t *crtc);
        int error_handler_(void);
        //bool has_randr_15_(Display *dpy_);
    };
} 

#endif 
