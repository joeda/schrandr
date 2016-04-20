#include <iostream>
#include <fstream>
#include <sstream>

#include "config.h"

namespace schrandr {
    
    JSONAdapter::JSONAdapter()
    {
    }

    void JSONAdapter::write_to_stream(std::stringstream *ofs, Json::Value content)
    {
        styled_stream_writer_.write(*ofs, content);
    }
    
    void JSONAdapter::write_to_stream(std::ostream *ofs, Json::Value content)
    {
        styled_stream_writer_.write(*ofs, content);
    }
    
    Json::Value JSONAdapter::read_stream(std::ifstream *str)
    {
        Json::Value res;
        bool parsed_success = reader_.parse(*str, res, false);
        return res;
    }
    
    Json::Value JSONAdapter::setup_to_json_(MonitorSetup ms)
    {
        Json::Value setup;
        Json::Value monitors;
        Json::Value screen;
        
        for(auto const& monitor_entry: ms.get_setup()) {
            Json::Value monitor;
            monitor["EDID"] = monitor_entry.get_edid();
            monitor["xres"] = 4;
            monitor["yres"] = 5;
            monitors.append(monitor);
        }
        setup["monitors"] = monitors;
        setup["screen"] = 0;
        return setup;
    }
    
    Mode JSONAdapter::mode_from_json(Json::Value root)
    {
        Mode m;
        Json::Value json_crtcs = root["crtcs"];
        if (json_crtcs.isArray()){
            Json::Value json_crtc;
            for (int i = 0; i < json_crtcs.size(); i++) {
                json_crtc = json_crtcs[i];
                CRTC crtc;
                int crtc_as_int;
                uint32_t crtc_as_uint;
                crtc_as_int = json_crtc["crtc"].asUInt();
                crtc_as_uint = static_cast<uint32_t>(crtc_as_int);
                crtc.crtc = static_cast<xcb_randr_crtc_t>(crtc_as_uint);
                
                Json::Value json_outputs = json_crtc["outputs"];
                Json::Value json_output;
                if (json_outputs.isArray()) {
                    for (int j = 0; j < json_outputs.size(); j++) {
                        json_output = json_outputs[j];
                        Output output;
                        output.x = json_output["x"].asInt();
                        output.y = json_output["y"].asInt();
                        int mode_as_int;
                        int output_as_int;
                        uint32_t mode_as_uint;
                        uint32_t output_as_uint;
                        
                        mode_as_int = json_output["mode"].asInt();
                        output_as_int = json_output["output"].asInt();
                        mode_as_uint = static_cast<uint32_t>(mode_as_int);
                        output_as_uint = static_cast<uint32_t>(output_as_int);
                        output.mode = static_cast<xcb_randr_mode_t>(mode_as_uint);
                        output.output = static_cast<xcb_randr_output_t>(output_as_uint);
                        crtc.outputs.push_back(output);
                    }
                }
                m.add_crtc(crtc);
            }
        }
        return m;
    }
    
    Json::Value JSONAdapter::mode_to_json(Mode m)
    {
        Json::Value root;
        Json::Value crtcs;
        for(auto const& crtc: m.get_crtcs()) {
            Json::Value json_crtc;
            Json::Value outputs;
            for(auto const& output: crtc.outputs) {
                Json::Value json_output;
                json_output["mode"] = reinterpret_cast<uint32_t>(output.mode);
                json_output["x"] = output.x;
                json_output["y"] = output.y;
                json_output["output"] = reinterpret_cast<uint32_t>(output.output);
                outputs.append(json_output);
            }
            json_crtc["outputs"] = outputs;
            json_crtc["crtc"] = reinterpret_cast<uint32_t>(crtc.crtc);
            crtcs.append(json_crtc);
        }
        root["crtcs"] = crtcs;
        
        return root;
    }
}   