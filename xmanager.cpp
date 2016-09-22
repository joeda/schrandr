#include <cstdlib>
#include <stdio.h>
#include <string>
#include <cstring>
#include <stdlib.h>
#include <inttypes.h>
#include <iomanip>

#include "xmanager.h"
#include "mode.h"
#include "edid.h"
#include "monitor_setup.h"
#include "defs.h"

#define OCNE(X) ((XRROutputChangeNotifyEvent*)X)
#define BUFFER_SIZE 128

namespace schrandr {
    
    XManager::XManager()
    {
        xcb_connection_ = xcb_connect(NULL, &screen_);
        get_screens_raw_();
        make_window_dummy_();
        get_outputs_raw_();
        
//         con_actions.push_back("connected");
//         con_actions.push_back("disconnected");
//         con_actions.push_back("unknown");
    }
    
    XManager::~XManager()
    {
        xcb_disconnect(xcb_connection_);
    }
    
    int XManager::error_handler_(void)
    {
        exit(1);
    }
    
    
//     bool XManager::has_randr_15_(Display *dpy)
//     {
//         int major, minor;
//         int event_base, error_base;
//         if (!XRRQueryExtension (dpy, &event_base, &error_base) ||
//             !XRRQueryVersion (dpy, &major, &minor))
//         {
//             fprintf (stderr, "RandR extension missing\n");
//             exit (EXIT_FAILURE);
//         }
//         if (major > 1 || (major == 1 && minor >= 5))
//             return true;
//         else
//             return false;
//     }
    
    
//     int predicate_event_(Display *display, XEvent *ev, XPointer arg) {
//         return true;
//     }
    
    void XManager::get_screens_raw_()
    {
        const xcb_setup_t *setup = xcb_get_setup(xcb_connection_);
        screens_ = xcb_setup_roots_iterator(setup).data;
        n_screens_ = xcb_setup_roots_length(setup);
    }
    
    void XManager::get_outputs_raw_()
    {
        xcb_randr_get_screen_resources_current_cookie_t screenResCookie = {};
        screenResCookie = xcb_randr_get_screen_resources_current(xcb_connection_, 
                                                     window_dummy_);

        //Receive reply from X server
        xcb_randr_get_screen_resources_current_reply_t* screenResReply = {};
        screenResReply = xcb_randr_get_screen_resources_current_reply(xcb_connection_,
                     screenResCookie, 0);
        
        if(screenResReply) {
            n_outputs_ = 
                xcb_randr_get_screen_resources_current_outputs_length(
                    screenResReply);
            outputs_ = 
                xcb_randr_get_screen_resources_current_outputs(screenResReply);
        }
    }
    
    bool XManager::get_outputs_raw_(xcb_randr_output_t *outputs, int *n_outputs)
    {
        xcb_randr_get_screen_resources_cookie_t screenResCookie;
        screenResCookie = xcb_randr_get_screen_resources_current(
            xcb_connection_, window_dummy_);

        //Receive reply from X server
        xcb_randr_get_screen_resources_reply_t* screenResReply;
        screenResReply = xcb_randr_get_screen_resources_reply(
            xcb_connection_, screenResCookie, 0);
        
        if(screenResReply) {
            *n_outputs = 
                xcb_randr_get_screen_resources_outputs_length(
                    screenResReply);
            outputs = 
                xcb_randr_get_screen_resources_outputs(screenResReply);
            return true;
        } else {
            *n_outputs = 0;
            outputs = nullptr;
            return false;
        }
    }
    
    void XManager::make_window_dummy_()
    {
        window_dummy_ = xcb_generate_id(xcb_connection_);
        xcb_create_window(xcb_connection_, 0, window_dummy_, screens_->root,
                      0, 0, 1, 1, 0, 0, 0, 0, 0);
        xcb_randr_select_input(xcb_connection_, window_dummy_,
                            XCB_RANDR_NOTIFY_MASK_SCREEN_CHANGE //89
                            | XCB_RANDR_NOTIFY_MASK_CRTC_CHANGE //90
                            );

        xcb_flush(xcb_connection_);
    }
    
    void XManager::refreshAll_()
    {
        get_screens_raw_();
        get_outputs_raw_();
    }
    
    void XManager::print_screen_info()
    {
        xcb_randr_get_screen_info_cookie_t screen_info_cookie = 
            xcb_randr_get_screen_info(xcb_connection_, window_dummy_);
        xcb_randr_get_screen_info_reply_t *screen_info_reply = 
            xcb_randr_get_screen_info_reply(
                xcb_connection_, screen_info_cookie, NULL);
        xcb_randr_screen_size_t *screen_sizes = 
            xcb_randr_get_screen_info_sizes(screen_info_reply);
        int n_sizes = 
            xcb_randr_get_screen_info_sizes_length (screen_info_reply);
        std::cout << "Screen sizeID: " 
            << std::to_string(screen_info_reply->sizeID) << std::endl;
        for (int i = 0; i < n_sizes; i++) {
            std::cout << "Size " << std::to_string(i) << std::endl;
            std::cout << "-------------------" << std::endl;
            std::cout << "\tWidth: " << 
                std::to_string(screen_sizes[i].width);
            std::cout << "\n\tHeight: " 
                << std::to_string(screen_sizes[i].height);
            std::cout << "\n\tMWidth: "
                << std::to_string(screen_sizes[i].mwidth);
            std::cout << "\n\tMHeight: "
                << std::to_string(screen_sizes[i].mheight);
            std::cout << "\n-----------------" << std::endl;
            std::cout << std::endl;
        }
        std::cout << "Size: " << std::to_string(screens_->width_in_pixels) << "x"
            << std::to_string(screens_->height_in_pixels) << std::endl;
        std::cout << "Size in mm: " 
            << std::to_string(screens_->width_in_millimeters) << "x"
            << std::to_string(screens_->height_in_millimeters) << std::endl;
        
        
    }
    
    void XManager::set_mode(Mode m)
    {
        for(auto const& screen: m.get_screens()) {
            xcb_randr_set_screen_size(xcb_connection_,
                    window_dummy_,
                    static_cast<uint16_t>(screen.width),
                    static_cast<uint16_t>(screen.height),
                    static_cast<uint32_t>(screen.width_mm),
                    static_cast<uint32_t>(screen.height_mm));
            for(auto const& crtc: screen.get_crtcs()) {
                Output output;
                output = crtc.outputs.front();
                xcb_randr_set_crtc_config_cookie_t crtc_config_cookie;
                crtc_config_cookie = xcb_randr_set_crtc_config (xcb_connection_,
                                    crtc.crtc,
                                    XCB_CURRENT_TIME,
                                    XCB_CURRENT_TIME,
                                    static_cast<int16_t>(output.x),
                                    static_cast<int16_t>(output.y),
                                    output.mode,
                                    XCB_RANDR_ROTATION_ROTATE_0, 
                                    1,
                                    &output.output);
                xcb_randr_set_crtc_config_reply_t *crtc_config_reply 
                    = xcb_randr_set_crtc_config_reply(xcb_connection_, crtc_config_cookie, NULL);
                std::cout << "Config response : " << std::to_string(crtc_config_reply->response_type) << std::endl;
                std::cout << "Config status : " << std::to_string(crtc_config_reply->status) << std::endl;
            }
        }
    }
    
    Edid XManager::get_edid_(const xcb_randr_output_t &output)
    {
        xcb_atom_t *output_properties_atoms;
        int n_atoms;
        //getting atom names
        xcb_get_atom_name_cookie_t atom_name_cookie;
        xcb_get_atom_name_reply_t *atom_name_reply;
        char *atom_name;
        int atom_name_length;
        //getting atom content
        xcb_randr_get_output_property_cookie_t property_cookie;
        xcb_randr_get_output_property_reply_t *property_reply;
        uint8_t *property_data;
        size_t property_data_length;
        xcb_randr_list_output_properties_reply_t *output_properties_reply;
        xcb_randr_list_output_properties_cookie_t output_properties_cookie;
        
        output_properties_cookie = xcb_randr_list_output_properties(
                xcb_connection_, output);
        output_properties_reply = xcb_randr_list_output_properties_reply (
                xcb_connection_, output_properties_cookie, NULL);
        output_properties_atoms = xcb_randr_list_output_properties_atoms(
            output_properties_reply);
        n_atoms = xcb_randr_list_output_properties_atoms_length(
            output_properties_reply);
        std::cout << "Gettin some EDID" << std::endl;
        for (int atom = 0; atom < n_atoms; atom++) {
            atom_name_cookie = xcb_get_atom_name(
                xcb_connection_, output_properties_atoms[atom]);
            atom_name_reply = xcb_get_atom_name_reply (
                xcb_connection_, atom_name_cookie, NULL);
            atom_name = xcb_get_atom_name_name(atom_name_reply);
            atom_name_length = xcb_get_atom_name_name_length(atom_name_reply);
            atom_name[atom_name_length] = '\0';
            
            if (!strcmp(atom_name, "EDID")) {
                property_cookie = xcb_randr_get_output_property(
                    xcb_connection_,
                    output,
                    output_properties_atoms[atom],
                    XCB_GET_PROPERTY_TYPE_ANY,
                    0,
                    100,
                    0,
                    0);
                property_reply = xcb_randr_get_output_property_reply(
                    xcb_connection_, property_cookie, NULL);
                property_data = xcb_randr_get_output_property_data(
                    property_reply);
                property_data_length = 
                    xcb_randr_get_output_property_data_length(
                        property_reply);
                return Edid(property_data, property_data_length);
            }
        }
        
        return Edid();
    }
    
    std::vector<std::string> XManager::getAvailableAtoms(
            const xcb_randr_output_t &output)
    {
        std::vector<std::string> res;
        
        auto output_properties_cookie = xcb_randr_list_output_properties(
                xcb_connection_, output);
        auto output_properties_reply = xcb_randr_list_output_properties_reply (
                xcb_connection_, output_properties_cookie, NULL);
        auto output_properties_atoms = xcb_randr_list_output_properties_atoms(
            output_properties_reply);
        int n_atoms = xcb_randr_list_output_properties_atoms_length(
            output_properties_reply);
        for (int atom = 0; atom < n_atoms; atom++) {
            auto atom_name_cookie = xcb_get_atom_name(
                xcb_connection_, output_properties_atoms[atom]);
            auto atom_name_reply = xcb_get_atom_name_reply (
                xcb_connection_, atom_name_cookie, NULL);
            char* atom_name = xcb_get_atom_name_name(atom_name_reply);
            int atom_name_length = xcb_get_atom_name_name_length(atom_name_reply);
            atom_name[atom_name_length] = '\0';
            res.push_back(std::string(atom_name));
        }
        
        return res;
    }
    
    Mode XManager::get_mode()
    {
        refreshAll_();
        Mode res;
        int n_screens = 1;
        const xcb_setup_t *setup = xcb_get_setup(xcb_connection_);
        xcb_screen_iterator_t iter = xcb_setup_roots_iterator(setup);  

        for (int i = 0; i < screen_; ++i) {
            xcb_screen_next(&iter);
        }

        xcb_screen_t *screens = iter.data;
        
        for (int s = 0; s < n_screens; s++) {
            Screen screen;
            screen.width = screens[s].width_in_pixels;
            screen.height = screens[s].height_in_pixels;
            screen.width_mm = screens[s].width_in_millimeters;
            screen.height_mm = screens[s].height_in_millimeters;
            
            xcb_randr_get_screen_resources_cookie_t screen_resources_cookie = 
                xcb_randr_get_screen_resources(xcb_connection_, window_dummy_);
            xcb_randr_get_screen_resources_reply_t *screen_resources_reply = 
                xcb_randr_get_screen_resources_reply(
                    xcb_connection_, screen_resources_cookie, NULL);
            
            xcb_randr_crtc_t *crtcs =  
                xcb_randr_get_screen_resources_crtcs(
                    screen_resources_reply);
            int n_crtcs = xcb_randr_get_screen_resources_crtcs_length(
                screen_resources_reply);
            
            char *cname;
            int n_name;
            
            //std::vector<CRTC> res.crtcs_;
            
            std::cout << "Found " << std::to_string(n_crtcs) << "CRTCs." << std::endl;
            for (int i = 0; i < n_crtcs; i++) {
                std::cout << "Debug A0" << std::endl;
                CRTC crtc;
                crtc.crtc = crtcs[i];
                //std::cout << "Now at CRTC " << std::to_string(crtcs[i]) << std::endl;
                xcb_randr_get_crtc_info_cookie_t crtc_info_cookie
                    = xcb_randr_get_crtc_info (
                        xcb_connection_, crtcs[i], XCB_CURRENT_TIME);
                xcb_randr_get_crtc_info_reply_t *crtc_info_reply =
                    xcb_randr_get_crtc_info_reply(
                        xcb_connection_, crtc_info_cookie, NULL);
                xcb_randr_output_t *outputs =
                    xcb_randr_get_crtc_info_outputs(crtc_info_reply);
                std::cout << "Debug A7" << std::endl;
                int n_outputs = xcb_randr_get_crtc_info_outputs_length(
                    crtc_info_reply);
                std::cout << "This CRTC has " << std::to_string(n_outputs) << " outputs." << std::endl;
                for (int o = 0; o < n_outputs; o++) {
                    std::cout << "Debug A1" << std::endl;
                    Output op;
                    op.output = outputs[o];
                    auto atoms = getAvailableAtoms(outputs[o]);
                    for (const auto &atom : atoms) {
                        std::cout << "Atom available in "
                                  << std::to_string(outputs[o])
                                  << ": " << atom << std::endl;
                    }
                    xcb_randr_get_output_info_cookie_t output_info_cookie =
                        xcb_randr_get_output_info(
                            xcb_connection_, outputs[o], XCB_CURRENT_TIME);
                    xcb_randr_get_output_info_reply_t *output_info_reply =
                        xcb_randr_get_output_info_reply(
                            xcb_connection_, output_info_cookie, XCB_CURRENT_TIME);
                    /* char* name = xcb_randr_get_output_info_name(output_info_reply);
                    n_name = xcb_randr_get_output_info_name_length(output_info_reply);
                    cname[n_name] = '\0'; */
                    std::cout << "Debug A2" << std::endl;
                    xcb_randr_mode_t * modes =
                        xcb_randr_get_output_info_modes(output_info_reply);
                    int n_modes = xcb_randr_get_output_info_modes_length (output_info_reply);
                    std::cout << "Debug A3" << std::endl;
                    op.mode = modes[0];
                    op.x = crtc_info_reply->x;
                    op.y = crtc_info_reply->y;
                    op.edid = get_edid_(outputs[o]);
                    crtc.outputs.push_back(op);
                    std::cout << "Debug A4" << std::endl;
                }
                std::cout << "Debug A8" << std::endl;
                if (n_outputs > 0) {
                    std::cout << "Debug A9" << std::endl;
                    screen.add_crtc(crtc);
                    std::cout << "Debug A10" << std::endl;
                }
            }
            std::cout << "Debug A5" << std::endl;
            res.add_screen(screen);
        }
        std::cout << "Debug A6" << std::endl;
        return res;
    }
    
    MonitorSetup XManager::get_monitors()
    {
        std::cout << "get_monitors() ----" << std::endl;
        MonitorSetup monitor_setup;
        int n_outputs;
        xcb_randr_output_t *outputs;
        if (!get_outputs_raw_(outputs, &n_outputs)) {}
            return monitor_setup;
        }
        printf("Number of outputs: %d\n", n_outputs);
        for (int i = 0; i < n_outputs; ++i) {
            // check if a device is connected
            auto outputInfoCookie = xcb_randr_get_output_info(
                xcb_connection_, outputs[i], XCB_CURRENT_TIME);
            xcb_randr_get_output_info_reply_t *reply = 
                xcb_randr_get_output_info_reply(xcb_connection_, reply, nullptr);
            char *output_name;
            output_name = strndup((char *) xcb_randr_get_output_info_name(reply),
                                  xcb_randr_get_output_info_name_length(reply));
            printf("Output Name: %s\n", output_name)
            std::cout << "Output Status: " << std::to_string(reply->connection)
                      << std::endl;
            Edid curEdid = get_edid_(outputs[i]);
            if (!curEdid.isDummy()) {
                monitor_setup.add_edid(curEdid);
            }
        }
        
        std::cout << "get_monitors() ----" << std::endl;
        
        return monitor_setup;
    }
    
    /** check_for_events()
     * the response_type for SCREEN_CHANGE is 90 and for
     * CRTC_CHANGE is 89
     * A change in modes, such as using xrandr, will yield both SCREEN_CHANGE
     * and CRTC_CHANGE. (Dis)connecting a monitor will yield only a CRTC_CHANGE.
     */
    
    schrandr_event_t XManager::check_for_events()
    {
        bool crtc_event = false;
        bool screen_event = false;
        xcb_generic_event_t *ev;
        while ((ev = xcb_poll_for_event(xcb_connection_)) != NULL) {
            std::cout << "Debug F1" << std::endl;
            if (ev->response_type == 0) {
                std::cout << "Response type 0" << std::endl;
                free(ev);
                continue;
            }
            std::cout << "Response Type: " << std::to_string(ev->response_type)
                << std::endl;
            std::cout << "Screen Change: " << std::to_string(XCB_RANDR_NOTIFY_MASK_SCREEN_CHANGE) << std::endl;
            std::cout << "Output Change: " << std::to_string(XCB_RANDR_NOTIFY_MASK_OUTPUT_CHANGE) << std::endl;
            std::cout << "CRTC Change: " << std::to_string(XCB_RANDR_NOTIFY_MASK_CRTC_CHANGE) << std::endl;
            std::cout << "Property Change: " << std::to_string(XCB_RANDR_NOTIFY_MASK_OUTPUT_PROPERTY) << std::endl;
            switch (static_cast<x_event_t>(ev->response_type)) {
            case SCREEN_CHANGE: {
                std::cout << "Screen change" << std::endl;
                screen_event = true;
                break;
            }
            case CRTC_CHANGE: {
                std::cout << "CRTC change" << std::endl;
                crtc_event = true;
                break;
            }
            default: {
                std::cout << "None of Screen, output or CRTC change" << std::endl;
                break;
            }
            }
            free(ev);
        }
        if (crtc_event && screen_event) {
            std::cout << "Returning MODE_EVENT" << std::endl;
            return MODE_EVENT;
        } else if (!crtc_event && screen_event) {
            std::cout << "Returning CONNECTION_EVENT" << std::endl;
            return CONNECTION_EVENT;
        } else {
            std::cout << "Returning OTHER_EVENT" << std::endl;
            return OTHER_EVENT;
        }
    }
}
