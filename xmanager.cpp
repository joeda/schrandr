#include "xmanager.h"
#include <cstdlib>
#include <stdio.h>
#include <string>
#include <cstring>
#include <stdlib.h>

#include <xcb/randr.h>

#define OCNE(X) ((XRROutputChangeNotifyEvent*)X)
#define BUFFER_SIZE 128

namespace schrandr {
    
    XManager::XManager()
    {
        xcb_connection_ = xcb_connect (NULL, NULL);
        if ((dpy_ = XOpenDisplay(NULL)) == NULL)
            std::cerr << "Could not open display";
        XRRSelectInput(dpy_, DefaultRootWindow(dpy_), RROutputChangeNotifyMask);
        XSync(dpy_, False);
        //XSetIOErrorHandler((XIOErrorHandler) error_handler_);
        screen_ = DefaultScreen (dpy_);
        root_ = RootWindow (dpy_, screen_);
        if (has_randr_15_(dpy_)) {
            XRRMonitorInfo* monitor_info = get_monitors_(dpy_, root_);
            std::vector<std::string> minfo =
                monitor_info_to_string_(monitor_info);
        }
        
        con_actions.push_back("connected");
        con_actions.push_back("disconnected");
        con_actions.push_back("unknown");
    }
    
    XManager::~XManager()
    {
        xcb_disconnect(xcb_connection_);
    }
    
    int XManager::error_handler_(void)
    {
        exit(1);
    }
    
    
    bool XManager::has_randr_15_(Display *dpy)
    {
        int major, minor;
        int event_base, error_base;
        if (!XRRQueryExtension (dpy, &event_base, &error_base) ||
            !XRRQueryVersion (dpy, &major, &minor))
        {
            fprintf (stderr, "RandR extension missing\n");
            exit (EXIT_FAILURE);
        }
        if (major > 1 || (major == 1 && minor >= 5))
            return true;
        else
            return false;
    }
    
    
    int predicate_event_(Display *display, XEvent *ev, XPointer arg) {
        return true;
    }
    
    std::vector<std::string> XManager::get_monitor_setup() {
        std::vector<std::string> monitors;
        const xcb_setup_t *setup;
        xcb_screen_iterator_t iter;
        xcb_screen_t *screen;
        
        setup = xcb_get_setup(xcb_connection_);
        iter = xcb_setup_roots_iterator(setup);
        screen = iter.data;
        std::string screen_res = "Screen X Res: ";
        screen_res += std::to_string(screen->width_in_pixels);
        screen_res += " Screen Y Res: ";
        screen_res += std::to_string(screen->height_in_pixels);
        monitors.push_back(screen_res);
        
        return monitors;
    }
    
    
    XRRMonitorInfo* XManager::get_monitors_(Display *dpy, Window root)
    {
        XRRMonitorInfo *m;
        int n;
        m = XRRGetMonitors(dpy, root, false, &n);
        if (n == -1) {
            fprintf(stderr, "get monitors failed\n");
            exit(EXIT_FAILURE);
        }
        return m;
    }
    
    XRandrMonitorInfo XManager::get_monitors()
    {
        XRandrMonitorInfo ret;
        ret.minfo = XRRGetMonitors(dpy_, root_, false, &ret.n_monitors);
        if (ret.n_monitors == -1) {
            fprintf(stderr, "get monitors failed\n");
            exit(EXIT_FAILURE);
        }
        
        return ret;
    }
    
    std::vector<std::string> XManager::monitor_info_to_string_
    (XRRMonitorInfo *monitor_info)
    {
        std::vector<std::string> info;
        info.push_back("---- Monitor Info ----");
        
        std::string x = "x: ";
        x += std::to_string(monitor_info->x);
        info.push_back(x);
        std::string y = "y: ";
        y += std::to_string(monitor_info->y);
        info.push_back(y);
        std::string noutput = "noutput: ";
        noutput += std::to_string(monitor_info->noutput);
        info.push_back(noutput);
        std::string width = "width: ";
        width += std::to_string(monitor_info->width);
        info.push_back(width);
        std::string height = "height: ";
        height += std::to_string(monitor_info->height);
        info.push_back(height);
        info.push_back("---- End Monitor Info ----");
        
        return info;
    }
    
    std::vector<std::string> XManager::get_monitor_info()
    {
        XRRMonitorInfo* minf = get_monitors_(dpy_, root_);
        std::vector<std::string> info = monitor_info_to_string_(minf);
        return info;
    }
    
    std::string XManager::get_edid()
    {
        std::string return_string = "";
        //Get the first X screen
        xcb_screen_t* XFirstScreen = xcb_setup_roots_iterator(
                               xcb_get_setup(xcb_connection_)).data;
        //Generate ID for the X window
        xcb_window_t XWindowDummy = xcb_generate_id(xcb_connection_);

        //Create dummy X window
        xcb_create_window(xcb_connection_, 0, XWindowDummy, XFirstScreen->root,
                      0, 0, 1, 1, 0, 0, 0, 0, 0);

        //Flush pending requests to the X server
        xcb_flush(xcb_connection_);

        //Send a request for screen resources to the X server
        xcb_randr_get_screen_resources_current_cookie_t screenResCookie = {};
        screenResCookie = xcb_randr_get_screen_resources_current(xcb_connection_, 
                                                     XWindowDummy);

        //Receive reply from X server
        xcb_randr_get_screen_resources_current_reply_t* screenResReply = {};
        screenResReply = xcb_randr_get_screen_resources_current_reply(xcb_connection_,
                     screenResCookie, 0);
        
        xcb_randr_output_t *outputs;
        int n_outputs = 0;
        
        if(screenResReply) {
            n_outputs = 
                xcb_randr_get_screen_resources_current_outputs_length(
                    screenResReply);
            outputs = 
                xcb_randr_get_screen_resources_current_outputs(screenResReply);
        } else
            return NULL;
                
        char *output_name;
        
        printf("Number of outputs: %d\n", n_outputs);
        
        xcb_randr_list_output_properties_cookie_t output_properties_cookie;
        xcb_randr_list_output_properties_reply_t *output_properties_reply;
        xcb_atom_t *output_properties_atoms;
        int n_atoms;
        xcb_get_atom_name_cookie_t atom_name_cookie;
        xcb_get_atom_name_reply_t *atom_name_reply;
        char *atom_name;
        
        for (int output = 0; output < n_outputs; output++) {
            printf("Not at output %d\n", output);
            xcb_randr_get_output_info_cookie_t output_info_cookie = xcb_randr_get_output_info(
                xcb_connection_,
                outputs[output],
                XCB_CURRENT_TIME);
            xcb_randr_get_output_info_reply_t *output_info =
                xcb_randr_get_output_info_reply(xcb_connection_,
                                                output_info_cookie,
                                                NULL);
            output_name = strndup((char *) xcb_randr_get_output_info_name(output_info),
                                  xcb_randr_get_output_info_name_length(output_info));
            printf("Output Name: %s\n", output_name);
            
            output_properties_cookie = xcb_randr_list_output_properties(
                xcb_connection_, outputs[output]);
            output_properties_reply = xcb_randr_list_output_properties_reply (
                xcb_connection_, output_properties_cookie, NULL);
            output_properties_atoms = xcb_randr_list_output_properties_atoms(
                output_properties_reply);
            n_atoms = xcb_randr_list_output_properties_atoms_length(
                output_properties_reply);
            printf("Number of atoms: %d\n", n_atoms);
            
            for (int atom = 0; atom < n_atoms; atom++) {
                atom_name_cookie = xcb_get_atom_name(
                    xcb_connection_, output_properties_atoms[atom]);
                atom_name_reply = xcb_get_atom_name_reply (
                    xcb_connection_, atom_name_cookie, NULL);
                atom_name = xcb_get_atom_name_name(atom_name_reply);
                printf("Atom Name: %s\n", atom_name);
            }
        
// //         for(int i = 0; i < n_atoms; i++) {
// //             if(atoms[i]) {
// //                 atom_name_cookie = xcb_get_atom_name(xcb_connection_, atoms[i]);
// //                 atom_name_reply = xcb_get_atom_name_reply(
// //                     xcb_connection_, atom_name_cookie, dummy_generic_error);
// //                 atom_name = xcb_get_atom_name_name (atom_name_reply);
// //                 atom_name_length =
// //                     xcb_get_atom_name_name_length(atom_name_reply);
// //                 printf("Atom Name Length: %d\n", atom_name_length);
// //                 printf("Atom Name: ");
// //                 std::cout << std::string(atom_name, atom_name_length);
// //                 printf("\n");
// //             }
// //         }
//             
//             unsigned char *ret = NULL;
//             xcb_randr_get_output_property_cookie_t cookie;
//             xcb_randr_get_output_property_reply_t *reply;
//             uint8_t pending = 0;
//             
//             std::cout << "Debug #1" << std::endl;
//             cookie =
//                 xcb_randr_get_output_property(xcb_connection_, //conn
//                                                         outputs[output], //output
//                                                         atoms[0], //property
//                                                         XCB_GET_PROPERTY_TYPE_ANY, //type
//                                                         0, //long_offset
//                                                         100, //long_length
//                                                         0, //delete
//                                                         0); //pending
//             std::cout << "Debug #2" << std::endl;
//             
//             reply =
//                 xcb_randr_get_output_property_reply(xcb_connection_, cookie, NULL);
//             if (reply)
//                 {
//                     std::cout << "Debug #4" << std::endl;
//                     if ((reply->type == XCB_ATOM_INTEGER) && (reply->format == 8))
//                     {
//                         std::cout << "Debug #5" << std::endl;
//                         ret = (unsigned char*)malloc(reply->num_items * sizeof(unsigned char));
//                         std::cout << "Debug #6" << std::endl;
//                         memcpy(ret, xcb_randr_get_output_property_data(reply),
//                                 (reply->num_items * sizeof(unsigned char)));
//                     }
//                     free(reply);
//                 }
//             printf("%u", ret);
//             std::cout << "Debug #3" << std::endl;
        }
        
        return return_string;
    }
    
    std::vector<std::string> XManager::get_X_events()
    {
        std::vector<std::string> events;
        char log_buf[BUFFER_SIZE];
        char buf[BUFFER_SIZE];
        
        int (*predicate_)(Display*, XEvent*, XPointer);
        predicate_ = &predicate_event_;

        if (XCheckIfEvent(dpy_, &ev_, predicate_, dummy_)) {
            XRRScreenResources *resources = XRRGetScreenResources(OCNE(&ev_)->display,
                                                                  OCNE(&ev_)->window);
            if (resources == NULL) {
                fprintf(stderr, "Could not get screen resources\n");
            }
            XRROutputInfo *info = XRRGetOutputInfo(OCNE(&ev_)->display, resources,
                                                   OCNE(&ev_)->output);
            if (info == NULL) {
                XRRFreeScreenResources(resources);
                fprintf(stderr, "Could not get output info\n");
            }
            snprintf(buf, BUFFER_SIZE, "%s %s", info->name,
                     con_actions[info->connection]);
            printf("Event: %s %s\n", info->name,
                   con_actions[info->connection]);
            snprintf(log_buf, BUFFER_SIZE, "Event: %s %s\n", info->name, 
                     con_actions[info->connection]);
            events.push_back(log_buf);
            printf("Time: %lu\n", info->timestamp);
            snprintf(log_buf, BUFFER_SIZE, "Time: %lu\n", info->timestamp);
            events.push_back(log_buf);
            if (info->crtc == 0) {
                printf("Size: %lumm x %lumm\n", info->mm_width, info->mm_height);
            }
            else {
                printf("CRTC: %lu\n", info->crtc);
                XRRCrtcInfo *crtc = XRRGetCrtcInfo(dpy_, resources, info->crtc);
                if (crtc != NULL) {
                    printf("Size: %dx%d\n", crtc->width, crtc->height);
                    XRRFreeCrtcInfo(crtc);
                }
            }
            XRRFreeScreenResources(resources);
            XRRFreeOutputInfo(info);
        } else {
            events.push_back("No XNextEvent");
        }
        
        return events;
    }
}
