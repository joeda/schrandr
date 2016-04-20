#ifndef CONFIG_H_
#define CONFIG_H_

#include <string>
#include <json/json.h>

#include "mode.h"
#include "jsonadapter.h"

namespace schrandr {
    
    class Config {
    public:
        Config();
        void write_mode(Mode m);
        Mode read_mode();
        void write();
        void read();
        void print_all();
        
    private:
        std::string config_file_path_;
        std::string empty_setups_path_;
        Json::Value setups_as_json_;
        JSONAdapter json_adapter_;
    };
} 

#endif 
