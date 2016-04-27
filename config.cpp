#include <iostream>
#include <fstream>
#include <sstream>

#include "config.h"
#include "mode.h"

namespace schrandr {
    
    Config::Config():
        config_file_path_("/tmp/schrandr.conf"),
        empty_setups_path_("/home/jo/dev/schrandr/empty_setups.json")
    {}

    void Config::write_modes(std::vector<Mode> modes)
    {
        std::ofstream ofs;
        ofs.open(config_file_path_, std::ofstream::out | std::ofstream::trunc);
        std::stringstream ss;
        ss.str(std::string());
        Json::Value json_modelist = json_adapter_.modes_to_json(modes);
        json_adapter_.write_to_stream(&ss, json_modelist);
        ofs << ss.rdbuf() << std::flush;
    }
    
    std::vector<Mode> Config::read_modes()
    {
        std::ifstream config_file(config_file_path_);
        if (!config_file.good())
            config_file.open(empty_setups_path_);
        Json::Value json_modes = json_adapter_.read_stream(&config_file);
        
        return json_adapter_.modes_from_json(json_modes);
    }
    
    void Config::print_mode(Mode m)
    {
        json_adapter_.write_to_stream(
            &std::cout, json_adapter_.mode_to_json(m));
    }
    
    void Config::print_modelist(std::vector<Mode> m)
    {
        json_adapter_.write_to_stream(
            &std::cout ,json_adapter_.modes_to_json(m));
    }
}  
