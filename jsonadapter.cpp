#include <iostream>
#include <fstream>
#include <sstream>

#include "config.h"
#include "mode.h"
#include "edid.h"
#include "defs.h"

namespace schrandr {

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
    
    MonitorSetup JSONAdapter::setup_from_json(Json::Value root)
    {
        MonitorSetup m;
        Json::Value json_edids = root["EDIDs"];
        if(json_edids.isArray()) {
            for (int i = 0; i < json_edids.size(); i++) {
                Edid e(json_edids[i].asString());
                m.add_edid(e);
            }
        }
        return m;
    }
    
    Json::Value JSONAdapter::setup_to_json_(MonitorSetup ms)
    {
        Json::Value root;
        Json::Value json_edids;
        
        for(auto const& edid: ms.get_edids()) {
            json_edids.append(edid.to_string());
        }
        root["EDIDs"] = json_edids;
        
        return root;
    }
    
    Mode JSONAdapter::mode_from_json(Json::Value root)
    {
        Mode m;
        Json::Value json_screens = root["screens"];
        if(json_screens.isArray()) {
            for (int h = 0; h < json_screens.size(); h++) {
                Json::Value json_screen = json_screens[h];
                Screen screen;
                screen.width = json_screen["width"].asInt();
                screen.height = json_screen["height"].asInt();
                screen.width_mm = json_screen["width_mm"].asInt();
                screen.height_mm = json_screen["height_mm"].asInt();
                Json::Value json_crtcs = json_screen["crtcs"];
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
                                output.edid.set_edid(json_output["EDID"].asString());
                                output.name = json_output["name"].asString();
                                crtc.outputs.push_back(output);
                            }
                        }
                        screen.add_crtc(crtc);
                    }
                }
                m.add_screen(screen);
            }
        }
        return m;
    }
    
    Json::Value JSONAdapter::mode_to_json(Mode m)
    {
        Json::Value root;
        Json::Value screens;
        for(auto const& screen: m.get_screens()) {
            Json::Value json_screen;
            json_screen["width"] = screen.width;
            json_screen["height"] = screen.height;
            json_screen["width_mm"] = screen.width_mm;
            json_screen["height_mm"] = screen.height_mm;
            Json::Value crtcs;
            for(auto const& crtc: screen.get_crtcs()) {
                Json::Value json_crtc;
                Json::Value outputs;
                for(auto const& output: crtc.outputs) {
                    Json::Value json_output;
                    json_output["mode"] = reinterpret_cast<uint32_t>(output.mode);
                    json_output["x"] = output.x;
                    json_output["y"] = output.y;
                    json_output["output"] = reinterpret_cast<uint32_t>(output.output);
                    json_output["EDID"] = output.edid.to_string();
                    json_output["name"] = output.name;
                    outputs.append(json_output);
                }
                json_crtc["outputs"] = outputs;
                json_crtc["crtc"] = reinterpret_cast<uint32_t>(crtc.crtc);
                crtcs.append(json_crtc);
            }
            json_screen["crtcs"] = crtcs;
            screens.append(json_screen);
        }
        root["screens"] = screens;
        return root;
    }
    
    ModeList JSONAdapter::modesFromJson(const Json::Value &root)
    {
        ModeList modes;
        Json::Value json_modes = root["Known modes"];
        if(json_modes.isArray()) {
            for (int i = 0; i < json_modes.size(); i++) {
                Json::Value monitorSetup = json_modes[i];
                MonitorSetup key;
                for (int j = 0; j < monitorSetup["connectedEDIDs"].size(); ++j) {
                    key.add_edid(Edid(monitorSetup["connectedEDIDs"][j].asString()));
                }
                for (int j = 0; j < monitorSetup["modes"].size(); ++j) {
                    auto name = monitorSetup["modes"][j]["name"].asString();
                    Mode mode = mode_from_json(monitorSetup["modes"][j]["mode"]);
                    modes.addNamedMode(key, mode, name);
                }
            }
        }
        return modes;
    }
    
    Json::Value JSONAdapter::modesToJson(const ModeList &modes)
    {
        Json::Value root;
        Json::Value json_modes;
        for(const auto &mode: modes.getModeList()) {
            Json::Value jsonMonitorSetup;
            for (const auto &edid : mode.first.get_edids()) {
                jsonMonitorSetup["connectedEDIDs"].append(
                    Json::Value(edid.to_string()));
            }
            for (const auto &namedMode : mode.second) {
                Json::Value jsonNamedMode;
                jsonNamedMode["name"] = namedMode.first;
                jsonNamedMode["mode"] = mode_to_json(namedMode.second);
                jsonMonitorSetup["modes"].append(jsonNamedMode);
            }
            json_modes.append(jsonMonitorSetup);
        }
        root["Known modes"] = json_modes;
        
        return root;
    }
}   
