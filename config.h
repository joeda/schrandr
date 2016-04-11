#ifndef CONFIG_H_
#define CONFIG_H_

#include <string>
#include <json/json.h>

#include "monitor_setup.h"

namespace schrandr {
    
    class Config {
    public:
        Config();
        void write();
        void add_setup(MonitorSetup ms);
        void read();
        void print_all();
        std::string to_string()const;
    private:
        std::vector<MonitorSetup> known_setups_;
        std::string config_file_path_;
        std::string empty_setups_path_;
        Json::Value setups_as_json_;
    };
} 

#endif
 
 
