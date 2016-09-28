#include <cstdlib>
#include <stdio.h>
#include <string>
#include <cstring>
#include <stdlib.h>
#include <inttypes.h>
#include <iomanip>
#include <unistd.h>
#include <map>
#include <sstream>

#include "xmanager.h"
#include "mode.h"
#include "edid.h"
#include "monitor_setup.h"
#include "defs.h"
#include "util.h"
#include "mode_change.h"

#define OCNE(X) ((XRROutputChangeNotifyEvent*)X)
#define BUFFER_SIZE 128

namespace schrandr {
    
    XManager::XManager(std::shared_ptr<Logger> logger)
    : logger_(logger)
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
    
    bool XManager::get_outputs_raw_(xcb_randr_output_t **outputs, int *n_outputs)
    {
        xcb_randr_get_screen_resources_cookie_t screenResCookie;
        screenResCookie = xcb_randr_get_screen_resources(
            xcb_connection_, window_dummy_);

        //Receive reply from X server
        xcb_randr_get_screen_resources_reply_t* screenResReply;
        screenResReply = xcb_randr_get_screen_resources_reply(
            xcb_connection_, screenResCookie, 0);
        
        if(screenResReply) {
            *n_outputs = 
                xcb_randr_get_screen_resources_outputs_length(
                    screenResReply);
            *outputs = 
                xcb_randr_get_screen_resources_outputs(screenResReply);
            return true;
        } else {
            *n_outputs = 0;
            *outputs = nullptr;
            return false;
        }
    }
    
    bool XManager::get_crtcs_raw_(xcb_randr_crtc_t **crtcs, int *nCrtcs)
    {
        auto cookie = xcb_randr_get_screen_resources(xcb_connection_, window_dummy_);
        auto reply = xcb_randr_get_screen_resources_reply(
            xcb_connection_, cookie, nullptr);
        if (reply) {
            *crtcs = xcb_randr_get_screen_resources_crtcs(reply);
            *nCrtcs = xcb_randr_get_screen_resources_crtcs_length(reply);
            return true;
        } else {
            *crtcs = nullptr;
            *nCrtcs = 0;
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
    
    std::vector<xcb_randr_output_t> XManager::getConnectedOutputs()
    {
        std::vector<xcb_randr_output_t> res;
        int n_outputs;
        xcb_randr_output_t *outputs;
        if (!get_outputs_raw_(&outputs, &n_outputs)) {
            return res;
        }
        for (int i = 0; i < n_outputs; ++i) {
            // check if a device is connected
            auto outputInfoCookie = xcb_randr_get_output_info(
                xcb_connection_, outputs[i], XCB_CURRENT_TIME);
            xcb_randr_get_output_info_reply_t *reply = 
                xcb_randr_get_output_info_reply(
                    xcb_connection_, outputInfoCookie, nullptr);
            switch(reply->connection) {
            case XCB_RANDR_CONNECTION_CONNECTED:
                res.push_back(outputs[i]);
                break;
            case XCB_RANDR_CONNECTION_DISCONNECTED:
                break;
            case XCB_RANDR_CONNECTION_UNKNOWN:
                logger_->log(LOG_WARNING, std::string("Could not determine "
                    " connection status of output ") + std::to_string(outputs[i]));
                break;
            default:
                logger_->log(LOG_ERR, "Invalid xcb_randr_connection_t");
            }
        }
        
        return res;
    }
    
    std::vector<xcb_randr_output_t> XManager::getActiveOutputs()
    {
        std::vector<xcb_randr_output_t> res;
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
        for (int i = 0; i < n_crtcs; i++) {
            xcb_randr_get_crtc_info_cookie_t crtc_info_cookie
                = xcb_randr_get_crtc_info (
                    xcb_connection_, crtcs[i], XCB_CURRENT_TIME);
            xcb_randr_get_crtc_info_reply_t *crtc_info_reply =
                xcb_randr_get_crtc_info_reply(
                    xcb_connection_, crtc_info_cookie, NULL);
            xcb_randr_output_t *outputs =
                xcb_randr_get_crtc_info_outputs(crtc_info_reply);
            int n_outputs = xcb_randr_get_crtc_info_outputs_length(
                crtc_info_reply);
            for (int o = 0; o < n_outputs; o++) {
                res.push_back(outputs[o]);
            }
        }
        
        return res;
    }
    
    bool
    XManager::getCrtcByOutput_(
        const xcb_randr_output_t &output, xcb_randr_crtc_t *crtc)
    {
        auto cookie = xcb_randr_get_output_info(
            xcb_connection_, output, XCB_CURRENT_TIME);
        auto reply = xcb_randr_get_output_info_reply(
            xcb_connection_, cookie, nullptr);
        xcb_randr_crtc_t *crtcs = xcb_randr_get_output_info_crtcs(reply);
        int nCrtcs = xcb_randr_get_output_info_crtcs_length(reply);
        for (int i = 0; i < nCrtcs; ++i) {
            auto opCookie = xcb_randr_get_crtc_info(
                xcb_connection_, crtcs[i], XCB_CURRENT_TIME);
            auto opReply = xcb_randr_get_crtc_info_reply(
                xcb_connection_, opCookie, nullptr);
            auto outputs = xcb_randr_get_crtc_info_outputs(opReply);
            int nOutputs = xcb_randr_get_crtc_info_outputs_length(opReply);
            for (int j = 0; i < nOutputs; ++i) {
                if (output == outputs[j]) {
                    *crtc = crtcs[i];
                    return true;
                }
            }
        }
        
        return false;
    }
    
    std::map<Edid, xcb_randr_output_t> XManager::getOuputsByEdid_()
    {
        std::map<Edid, xcb_randr_output_t> res;
        
        auto cookie = xcb_randr_get_screen_resources(xcb_connection_, window_dummy_);
        auto reply = xcb_randr_get_screen_resources_reply(
            xcb_connection_, cookie, nullptr);
        xcb_randr_output_t *outputs = xcb_randr_get_screen_resources_outputs(reply);
        int nOutputs = xcb_randr_get_screen_resources_outputs_length(reply);
        for (int i = 0; i < nOutputs; ++i) {
            Edid edid = get_edid_(outputs[i]);
            if (!edid.to_string().empty()) {
                res[edid] = outputs[i];
            }
        }
        
        return res;
    }
    
    std::map<Edid, std::vector<xcb_randr_crtc_t> > XManager::getCrtcsByEdid_()
    {
        std::map<Edid, std::vector<xcb_randr_crtc_t> >res;
        int nCrtcs;
        xcb_randr_crtc_t *crtcs;
        get_crtcs_raw_(&crtcs, &nCrtcs);
        ///std::cout << "in getCrtcsByEdid: Found " << nCrtcs << " CRTCs" << std::endl;
        for (int i = 0; i < nCrtcs; ++i) {
            auto opCookie = xcb_randr_get_crtc_info(
                xcb_connection_, crtcs[i], XCB_CURRENT_TIME);
            auto opReply = xcb_randr_get_crtc_info_reply(
                xcb_connection_, opCookie, nullptr);
            auto outputs = xcb_randr_get_crtc_info_possible(opReply);
            int nOutputs = xcb_randr_get_crtc_info_possible_length(opReply);
            /// std::cout << "in getCrtcsByEdid: Found " << nOutputs << " Outputs" << std::endl;

            for (int j = 0; j < nOutputs; ++j) {
                auto edid = get_edid_(outputs[j]);
                if (!edid.to_string().empty()) {
                    /* std::cout << "Matched EDID " << edid.to_string()
                              << " to CRTC " << crtcs[i] << std::endl; */
                    res[edid].push_back(crtcs[i]);
                }
            }
        }
        
        return res;
    }
    
    std::vector<xcb_randr_mode_t>
    XManager::getAvailableModesFromOutput(const xcb_randr_output_t &output)
    {
        std::vector<xcb_randr_mode_t> res;
        auto cookie = xcb_randr_get_output_info(
            xcb_connection_, output, XCB_CURRENT_TIME);
        auto reply = xcb_randr_get_output_info_reply(
            xcb_connection_, cookie, nullptr);
        xcb_randr_mode_t *modes = xcb_randr_get_output_info_modes(reply);
        int nModes = xcb_randr_get_output_info_modes_length(reply);
        for (int i = 0; i < nModes; ++i) {
            res.push_back(modes[i]);
        }
        
        return res;
    }
    
    std::vector<xcb_randr_mode_info_t> XManager::getModeInfos()
    {
        std::vector<xcb_randr_mode_info_t> res;
        auto cookie = xcb_randr_get_screen_resources(
            xcb_connection_, window_dummy_);
        auto reply = xcb_randr_get_screen_resources_reply (
            xcb_connection_, cookie, nullptr);
        xcb_randr_mode_info_t *modeInfos = xcb_randr_get_screen_resources_modes(
            reply);
        int nModeInfos = xcb_randr_get_screen_resources_modes_length(reply);
        for (int i = 0; i < nModeInfos; ++i) {
            res.push_back(modeInfos[i]);
        }
        
        return res;
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
    
    bool XManager::set_mode(const Mode &current, const Mode &target)
    {
        bool success = true;
        ModeChange modeChange(current,
                              target,
                              getCrtcsByEdid_(),
                              getOuputsByEdid_());
        for(auto const& screen: modeChange.getScreenChanges()) {
            xcb_randr_set_screen_size(xcb_connection_,
                    window_dummy_,
                    static_cast<uint16_t>(screen.width),
                    static_cast<uint16_t>(screen.height),
                    static_cast<uint32_t>(screen.width_mm),
                    static_cast<uint32_t>(screen.height_mm));
        }
        for (const auto &crtc : modeChange.getCrtcsToDisable()) {
            disableCrtc(crtc);
        }
        for(auto const& crtc: modeChange.getCrtcChanges()) {
            std::vector<xcb_randr_output_t> outputs;
            for (const auto &op : crtc.outputs) {
                outputs.push_back(op.output);
            }
            xcb_randr_output_t* outputPtr = &outputs[0];
            std::ostringstream os;
            os << "XCB Call:" << std::endl;
            os << "\tCRTC: " << crtc.crtc
                    << "\n\tx: " << static_cast<int16_t>(crtc.x)
                    << "\n\ty: " << static_cast<int16_t>(crtc.y)
                    << "\n\tmode: " << crtc.mode
                    << "\n\toutput: " << outputs[0] << std::endl;
            logger_->log(std::string("Attempting XCB Call\n") + os.str());
            xcb_randr_set_crtc_config_cookie_t crtc_config_cookie;
            crtc_config_cookie = xcb_randr_set_crtc_config (xcb_connection_,
                                crtc.crtc,
                                XCB_CURRENT_TIME,
                                XCB_CURRENT_TIME,
                                static_cast<int16_t>(crtc.x),
                                static_cast<int16_t>(crtc.y),
                                crtc.mode,
                                XCB_RANDR_ROTATION_ROTATE_0, 
                                outputs.size(),
                                outputPtr);
            xcb_randr_set_crtc_config_reply_t *crtc_config_reply 
                = xcb_randr_set_crtc_config_reply(xcb_connection_, crtc_config_cookie, NULL);
            int responseType = crtc_config_reply->response_type;
            int status = crtc_config_reply->status;
            if (responseType != 1 || status != 0) {
                success = false;
                logger_->log(LOG_ERR, std::string("XCB Call\n")
                    + os.str() + "\nfailed");
            }
        }
        
        return success;
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
            
            //std::cout << "Found " << std::to_string(n_crtcs) << "CRTCs." << std::endl;
            for (int i = 0; i < n_crtcs; i++) {
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
                int n_outputs = xcb_randr_get_crtc_info_outputs_length(
                    crtc_info_reply);
                crtc.x = crtc_info_reply->x;
                crtc.y = crtc_info_reply->y;
                //std::cout << "This CRTC has " << std::to_string(n_outputs) << " outputs." << std::endl;
                for (int o = 0; o < n_outputs; o++) {
                    Output op;
                    op.output = outputs[o];
                    xcb_randr_get_output_info_cookie_t output_info_cookie =
                        xcb_randr_get_output_info(
                            xcb_connection_, outputs[o], XCB_CURRENT_TIME);
                    xcb_randr_get_output_info_reply_t *output_info_reply =
                        xcb_randr_get_output_info_reply(
                            xcb_connection_, output_info_cookie, XCB_CURRENT_TIME);
                    char* name = reinterpret_cast<char*>(
                        xcb_randr_get_output_info_name(output_info_reply));
                    int n_name = xcb_randr_get_output_info_name_length(
                        output_info_reply);
                    name[n_name] = '\0';
                    xcb_randr_mode_t * modes =
                        xcb_randr_get_output_info_modes(output_info_reply);
                    int n_modes = xcb_randr_get_output_info_modes_length (output_info_reply);
                    crtc.mode = modes[0];
                    op.edid = get_edid_(outputs[o]);
                    op.name = std::string(name);
                    crtc.outputs.push_back(op);
                }
                if (n_outputs > 0) {
                    screen.add_crtc(crtc);
                }
            }
            res.add_screen(screen);
        }
        return res;
    }
    
    MonitorSetup XManager::get_monitors()
    {
        //std::cout << "get_monitors() ----" << std::endl;
        MonitorSetup monitor_setup;
        int n_outputs;
        xcb_randr_output_t *outputs;
        if (!get_outputs_raw_(&outputs, &n_outputs)) {
            return monitor_setup;
        }
        //printf("Number of outputs: %d\n", n_outputs);
        for (int i = 0; i < n_outputs; ++i) {
            // check if a device is connected
            auto outputInfoCookie = xcb_randr_get_output_info(
                xcb_connection_, outputs[i], XCB_CURRENT_TIME);
            xcb_randr_get_output_info_reply_t *reply = 
                xcb_randr_get_output_info_reply(
                    xcb_connection_, outputInfoCookie, nullptr);
            switch(reply->connection) {
            case XCB_RANDR_CONNECTION_CONNECTED: {
                Edid curEdid = get_edid_(outputs[i]);
                monitor_setup.add_edid(curEdid);
            } break;
            case XCB_RANDR_CONNECTION_DISCONNECTED:
                break;
            case XCB_RANDR_CONNECTION_UNKNOWN: {
                std::cout << "Connection status unknown?" << std::endl;
            } break;
            default: {
                std::cout << "Something went seriously wrong" << std::endl;
            } break;
            }
        }
        
        //std::cout << "get_monitors() ----" << std::endl;
        
        return monitor_setup;
    }
    
    /** check_for_events()
     * the response_type for SCREEN_CHANGE is 90 and for
     * CRTC_CHANGE is 89
     * A change in modes, such as using xrandr, will yield both SCREEN_CHANGE
     * and CRTC_CHANGE. (Dis)connecting a monitor will yield only a CRTC_CHANGE.
     */
    bool XManager::disableOutput(
        const xcb_randr_output_t &output, const Mode &lastConnected)
    {
        auto modeCrtc = lastConnected.getCrtcByOutput(output);
        std::cout << "disableOutput: CRTC " << std::to_string(modeCrtc.crtc)
                  << " for output " << std::to_string(output) << std::endl;
        auto cookie = xcb_randr_set_crtc_config(
            xcb_connection_,
            modeCrtc.crtc,
            XCB_CURRENT_TIME,
            XCB_CURRENT_TIME,
            1, //x
            1, //y
            0, //Mode 
            XCB_RANDR_ROTATION_ROTATE_0, //rotation
            0, //outputs_len
            nullptr); //ptr to outputs
        auto reply = xcb_randr_set_crtc_config_reply(
            xcb_connection_, cookie, nullptr);
        std::cout << "disableOutpt response: " << std::to_string(reply->response_type)
                  << std::endl;
        //std::cout << "disableOutpt status: " << std::to_string(reply->status)
         //         << std::endl;
        //set_mode(modePre);  //fails for some reason
        return true;
    }
    
    bool XManager::disableCrtc(const CRTC &crtc) {
        auto cookie = xcb_randr_set_crtc_config(
            xcb_connection_,
            crtc.crtc,
            XCB_CURRENT_TIME,
            XCB_CURRENT_TIME,
            0, //x
            0, //y
            0, //Mode 
            XCB_RANDR_ROTATION_ROTATE_0, //rotation
            0, //outputs_len
            nullptr); //ptr to outputs
        auto reply = xcb_randr_set_crtc_config_reply(
            xcb_connection_, cookie, nullptr);
        std::cout << "disableCrtc response: " << std::to_string(reply->response_type)
                  << std::endl;
        //std::cout << "disableOutpt status: " << std::to_string(reply->status)
         //         << std::endl;
        //set_mode(modePre);  //fails for some reason
        return true;
    }
    
    bool XManager::disableCrtc(const xcb_randr_crtc_t &crtc) {
        auto cookie = xcb_randr_set_crtc_config(
            xcb_connection_,
            crtc,
            XCB_CURRENT_TIME,
            XCB_CURRENT_TIME,
            0, //x
            0, //y
            0, //Mode 
            XCB_RANDR_ROTATION_ROTATE_0, //rotation
            0, //outputs_len
            nullptr); //ptr to outputs
        auto reply = xcb_randr_set_crtc_config_reply(
            xcb_connection_, cookie, nullptr);
        std::cout << "disableCrtc response: " << std::to_string(reply->response_type)
                  << std::endl;
        //std::cout << "disableOutpt status: " << std::to_string(reply->status)
         //         << std::endl;
        //set_mode(modePre);  //fails for some reason
        return true;
    }
    
    bool XManager::activateAnyOutput()
    {
        std::cout << "Trying to activate any output" << std::endl;
        xcb_randr_crtc_t *crtcs;
        int nCrtcs;
        if (!get_crtcs_raw_(&crtcs, &nCrtcs)) {
            return false;
        }
        for (int i = 0; i < nCrtcs; ++i) {
            auto cookie = xcb_randr_get_crtc_info(
                xcb_connection_, crtcs[i], XCB_CURRENT_TIME);
            auto reply = xcb_randr_get_crtc_info_reply(
                xcb_connection_, cookie, nullptr);
            int nPossibleOutputs
                = xcb_randr_get_crtc_info_possible_length(reply);
            if (nPossibleOutputs > 0) {
                auto outputs = xcb_randr_get_crtc_info_possible (reply);
                auto modes = getAvailableModesFromOutput(outputs[0]);
                auto setCookie = xcb_randr_set_crtc_config(
                    xcb_connection_,
                    crtcs[i],
                    XCB_CURRENT_TIME,
                    XCB_CURRENT_TIME,
                    0, //x
                    0, //y
                    modes.front(), //Mode 
                    XCB_RANDR_ROTATION_ROTATE_0, //rotation
                    1, //outputs_len
                    outputs); //ptr to outputs
                auto setReply = xcb_randr_set_crtc_config_reply(
                    xcb_connection_, setCookie, nullptr);
                std::cout << "activateAny response: " << std::to_string(reply->response_type)
                        << std::endl;
                //std::cout << "activateAny status: " << std::to_string(reply->status)
                //        << std::endl;
                if (reply->response_type == 1) {
                    return true;
                }
            }
        }
        return false;
    }
    
    schrandr_event_t XManager::check_for_events()
    {
        bool crtc_event = false;
        bool screen_event = false;
        xcb_generic_event_t *ev;
        while ((ev = xcb_poll_for_event(xcb_connection_)) != NULL) {
            if (ev->response_type == 0) {
                std::cout << "Response type 0" << std::endl;
                free(ev);
                continue;
            }
            /* std::cout << "Response Type: " << std::to_string(ev->response_type)
                << std::endl;
            std::cout << "Screen Change: " << std::to_string(XCB_RANDR_NOTIFY_MASK_SCREEN_CHANGE) << std::endl;
            std::cout << "Output Change: " << std::to_string(XCB_RANDR_NOTIFY_MASK_OUTPUT_CHANGE) << std::endl;
            std::cout << "CRTC Change: " << std::to_string(XCB_RANDR_NOTIFY_MASK_CRTC_CHANGE) << std::endl;
            std::cout << "Property Change: " << std::to_string(XCB_RANDR_NOTIFY_MASK_OUTPUT_PROPERTY) << std::endl; */
            switch (static_cast<x_event_t>(ev->response_type)) {
            case SCREEN_CHANGE: {
                //std::cout << "Screen change" << std::endl;
                screen_event = true;
                break;
            }
            case CRTC_CHANGE: {
                //std::cout << "CRTC change" << std::endl;
                crtc_event = true;
                break;
            }
            default: {
                //std::cout << "None of Screen, output or CRTC change" << std::endl;
                break;
            }
            }
            free(ev);
        }
        if (crtc_event && screen_event) {
            return MODE_EVENT;
        } else if (!crtc_event && screen_event) {
            return CONNECTION_EVENT;
        } else {
            return OTHER_EVENT;
        }
    }
}
