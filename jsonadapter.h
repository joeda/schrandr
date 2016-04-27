#ifndef JSONADAPTER_H_
#define JSONADAPTER_H_

#include <json/json.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#include "monitor_setup.h"


namespace schrandr {
    
    class JSONAdapter {
    public:
        JSONAdapter();
        MonitorSetup setup_from_json(Json::Value root);
        Json::Value setup_to_json(MonitorSetup ms);
        Mode mode_from_json(Json::Value root);
        Json::Value mode_to_json(Mode m);
        std::vector<Mode> modes_from_json(Json::Value root);
        Json::Value modes_to_json(std::vector<Mode> modes);
        
        Json::Value read_stream(std::ifstream *str);
        void write_to_stream(std::stringstream *ofs, Json::Value content);
        void write_to_stream(std::ostream *ofs, Json::Value content);
        
    private:
        Json::StyledStreamWriter styled_stream_writer_;
        Json::Reader reader_;
        Json::Value setup_to_json_(MonitorSetup ms);
    };
} 

#endif 
 
