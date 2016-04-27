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
        xcb_connection_ = xcb_connect (NULL, NULL);
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
    
    Edid XManager::get_edid_(xcb_randr_output_t *output)
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
                xcb_connection_, *output);
        output_properties_reply = xcb_randr_list_output_properties_reply (
                xcb_connection_, output_properties_cookie, NULL);
        output_properties_atoms = xcb_randr_list_output_properties_atoms(
            output_properties_reply);
        n_atoms = xcb_randr_list_output_properties_atoms_length(
            output_properties_reply);
        
        std::cout << "Debug D9" << std::endl;
        for (int atom = 0; atom < n_atoms; atom++) {
            atom_name_cookie = xcb_get_atom_name(
                xcb_connection_, output_properties_atoms[atom]);
            atom_name_reply = xcb_get_atom_name_reply (
                xcb_connection_, atom_name_cookie, NULL);
            std::cout << "Debug D10" << std::endl;
            atom_name = xcb_get_atom_name_name(atom_name_reply);
            atom_name_length = xcb_get_atom_name_name_length(atom_name_reply);
            atom_name[atom_name_length] = '\0';
            printf("Atom Name: %s\n", atom_name);
            
            if (!strcmp(atom_name, "EDID")) {
                property_cookie = xcb_randr_get_output_property(
                    xcb_connection_,
                    *output,
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
                std::cout << "EDID: ";
                for (int i = 0; i < property_data_length; i++) {
                    std::cout << std::setw(2) << std::setfill('0') << std::hex << (int) property_data[i];
                }
                std::cout << std::endl;
                return Edid(property_data, property_data_length);
            }
        }
    }
    
    Mode XManager::get_mode()
    {
        Mode res;
        
        for (int s = 0; s < n_screens_; s++) {
            Screen screen;
            screen.width = screens_[s].width_in_pixels;
            screen.height = screens_[s].height_in_pixels;
            screen.width_mm = screens_[s].width_in_millimeters;
            screen.height_mm = screens_[s].height_in_millimeters;
        
            xcb_randr_mode_t *modes;
            int n_modes;
            
            xcb_randr_get_screen_resources_current_cookie_t screen_resources_cookie = 
                xcb_randr_get_screen_resources_current(xcb_connection_, window_dummy_);
            xcb_randr_get_screen_resources_current_reply_t *screen_resources_reply = 
                xcb_randr_get_screen_resources_current_reply(
                    xcb_connection_, screen_resources_cookie, NULL);
            
            xcb_randr_crtc_t *crtcs 
                = xcb_randr_get_screen_resources_current_crtcs(screen_resources_reply);
            int n_crtcs = xcb_randr_get_screen_resources_current_crtcs_length(screen_resources_reply);
            
            xcb_randr_get_crtc_info_cookie_t crtc_info_cookie;
            xcb_randr_get_crtc_info_reply_t *crtc_info_reply;
            xcb_randr_output_t *outputs;
            int n_outputs;
            xcb_randr_get_output_info_cookie_t output_info_cookie;
            xcb_randr_get_output_info_reply_t *output_info_reply;
            uint8_t *name;
            char *cname;
            int n_name;
            
            //std::vector<CRTC> res.crtcs_;
            
            std::cout << "Found " << std::to_string(n_crtcs) << "CRTCs." << std::endl;
            for (int i = 0; i < n_crtcs; i++) {
                CRTC crtc;
                crtc.crtc = crtcs[i];
                //std::vector<Output> crtc.outputs;
                std::cout << "Now at CRTC " << std::to_string(crtcs[i]) << std::endl;
                crtc_info_cookie = xcb_randr_get_crtc_info (
                    xcb_connection_, crtcs[i], XCB_CURRENT_TIME);
                crtc_info_reply = xcb_randr_get_crtc_info_reply(
                    xcb_connection_, crtc_info_cookie, NULL);
                outputs = xcb_randr_get_crtc_info_outputs(crtc_info_reply);
                n_outputs = xcb_randr_get_crtc_info_outputs_length(crtc_info_reply);
                std::cout << "This CRTC has " << std::to_string(n_outputs) << " outputs." << std::endl;
                for (int o = 0; o < n_outputs; o++) {
                    Output op;
                    op.output = outputs[o];
                    output_info_cookie = xcb_randr_get_output_info(
                        xcb_connection_, outputs[o], XCB_CURRENT_TIME);
                    output_info_reply = xcb_randr_get_output_info_reply(
                        xcb_connection_, output_info_cookie, XCB_CURRENT_TIME);
                    name = xcb_randr_get_output_info_name(output_info_reply);
                    n_name = xcb_randr_get_output_info_name_length(output_info_reply);
                    cname = reinterpret_cast<char*>(name);
                    cname[n_name] = '\0';
                    printf("Output %i: %s\n", o, cname);
                    
                    modes = xcb_randr_get_output_info_modes(output_info_reply);
                    n_modes = xcb_randr_get_output_info_modes_length (output_info_reply);
                    op.mode = modes[0];
                    op.x = crtc_info_reply->x;
                    op.y = crtc_info_reply->y;
                    op.edid = get_edid_(&outputs[o]);
                    crtc.outputs.push_back(op);
                }
                if (n_outputs > 0) {
                    screen.add_crtc(crtc);
                }
            }
            std::cout << "Debug F2" << std::endl;
            res.add_screen(screen);
        }
        std::cout << "Debug F3" << std::endl;
        return res;
    }
    
    MonitorSetup XManager::get_monitors()
    {
        std::cout << "get_monitors() ----" << std::endl;
        MonitorSetup monitor_setup;
        char *output_name;
        
        printf("Number of outputs: %d\n", n_outputs_);
        
        //getting output properties atoms
        xcb_randr_list_output_properties_cookie_t output_properties_cookie;
        xcb_randr_list_output_properties_reply_t *output_properties_reply;
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
        
        for (int output = 0; output < n_outputs_; output++) {
            printf("Not at output %d\n", output);
            xcb_randr_get_output_info_cookie_t output_info_cookie = xcb_randr_get_output_info(
                xcb_connection_,
                outputs_[output],
                XCB_CURRENT_TIME);
            xcb_randr_get_output_info_reply_t *output_info =
                xcb_randr_get_output_info_reply(xcb_connection_,
                                                output_info_cookie,
                                                NULL);
            output_name = strndup((char *) xcb_randr_get_output_info_name(output_info),
                                  xcb_randr_get_output_info_name_length(output_info));
            printf("Output Name: %s\n", output_name);
            
            output_properties_cookie = xcb_randr_list_output_properties(
                xcb_connection_, outputs_[output]);
            output_properties_reply = xcb_randr_list_output_properties_reply (
                xcb_connection_, output_properties_cookie, NULL);
            output_properties_atoms = xcb_randr_list_output_properties_atoms(
                output_properties_reply);
            n_atoms = xcb_randr_list_output_properties_atoms_length(
                output_properties_reply);
            
            for (int atom = 0; atom < n_atoms; atom++) {
                atom_name_cookie = xcb_get_atom_name(
                    xcb_connection_, output_properties_atoms[atom]);
                atom_name_reply = xcb_get_atom_name_reply (
                    xcb_connection_, atom_name_cookie, NULL);
                atom_name = xcb_get_atom_name_name(atom_name_reply);
                atom_name_length = xcb_get_atom_name_name_length(atom_name_reply);
                atom_name[atom_name_length] = '\0';
                printf("Atom Name: %s\n", atom_name);
                
                if (!strcmp(atom_name, "EDID")) {
                    property_cookie = xcb_randr_get_output_property(
                        xcb_connection_,
                        outputs_[output],
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
                    std::cout << "EDID: ";
                    for (int i = 0; i < property_data_length; i++) {
                        std::cout << std::setw(2) << std::setfill('0') << std::hex << (int) property_data[i];
                    }
                    std::cout << std::endl;
                    monitor_setup.add_edid(
                        Edid(property_data, property_data_length));
                }
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
        while ((ev_ = xcb_poll_for_event(xcb_connection_)) != NULL) {
            std::cout << "Debug F1" << std::endl;
            if (ev_->response_type == 0) {
                std::cout << "Response type 0" << std::endl;
                free(ev_);
                continue;
            }
            std::cout << "Response Type: " << std::to_string(ev_->response_type)
                << std::endl;
            std::cout << "Screen Change: " << std::to_string(XCB_RANDR_NOTIFY_MASK_SCREEN_CHANGE) << std::endl;
            std::cout << "Output Change: " << std::to_string(XCB_RANDR_NOTIFY_MASK_OUTPUT_CHANGE) << std::endl;
            std::cout << "CRTC Change: " << std::to_string(XCB_RANDR_NOTIFY_MASK_CRTC_CHANGE) << std::endl;
            std::cout << "Property Change: " << std::to_string(XCB_RANDR_NOTIFY_MASK_OUTPUT_PROPERTY) << std::endl;
            switch (static_cast<x_event_t>(ev_->response_type)) {
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
        free(ev_);
        }
        if (crtc_event && screen_event)
            return MODE_EVENT;
        else if (crtc_event && !screen_event)
            return CONNECTION_EVENT;
        else
            return OTHER_EVENT;
    }
}
