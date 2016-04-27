#ifndef CONFIG_H_
#define CONFIG_H_

#include <string>
#include <json/json.h>
#include <vector>

#include "mode.h"
#include "jsonadapter.h"

namespace schrandr {
    
    class Config {
    public:
        Config();
        void write_modes(std::vector<Mode> modes);
        std::vector<Mode> read_modes();
        void print_mode(Mode m);
        void print_modelist(std::vector<Mode> m);
        
    private:
        std::string config_file_path_;
        std::string empty_setups_path_;
        JSONAdapter json_adapter_;
    };
} 

#endif 
