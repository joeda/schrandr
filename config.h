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
        void write_modes(const ModeList &modes);
        ModeList read_modes();
        void print_mode(Mode m);
        void print_modelist(const ModeList &modes);
        
    private:
        std::string config_file_path_;
        std::string empty_setups_path_;
        JSONAdapter json_adapter_;
    };
} 

#endif 
