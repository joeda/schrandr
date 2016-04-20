#ifndef JSONADAPTER_H_
#define JSONADAPTER_H_

#include <json/json.h>
#include <iostream>
#include <fstream>
#include <sstream>

#include "monitor_setup.h"


namespace schrandr {
    
    class JSONAdapter {
    public:
        JSONAdapter();
        Json::Value setup_to_json(MonitorSetup ms);
        void write_to_stream(std::stringstream *ofs, Json::Value content);
        void write_to_stream(std::ostream *ofs, Json::Value content);
        Json::Value read_stream(std::ifstream *str);
        Mode mode_from_json(Json::Value root);
        Json::Value mode_to_json(Mode m);
        
    private:
        Json::StyledStreamWriter styled_stream_writer_;
        Json::Reader reader_;
        Json::Value setup_to_json_(MonitorSetup ms);
    };
} 

#endif 
 
