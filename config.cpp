#include <iostream>
#include <fstream>
#include <sstream>

#include "config.h"
#include "mode.h"

namespace schrandr {
    
    Config::Config():
        config_file_path_("/tmp/schrandr.conf"),
        empty_setups_path_("/home/jo/dev/schrandr/empty_setups.json")
    {
        read();
    }

    void Config::write()
    {
        std::ofstream ofs;
        ofs.open(config_file_path_, std::ofstream::out | std::ofstream::trunc);
        std::stringstream ss;
        ss.str(std::string());
        json_adapter_.write_to_stream(&ss, setups_as_json_);
        ofs << ss.rdbuf() << std::flush;
    }
    
    void Config::read()
    {
        std::ifstream config_file(config_file_path_);
        if (!config_file.good())
            config_file.open(empty_setups_path_);
        setups_as_json_ = json_adapter_.read_stream(&config_file);
    }
    
    Mode Config::read_mode()
    {
        std::ifstream config_file(config_file_path_);
        Json::Value json_mode = json_adapter_.read_stream(&config_file);
        return json_adapter_.mode_from_json(json_mode);
    }
    
    void Config::print_all() 
    {
        json_adapter_.write_to_stream(&std::cout, setups_as_json_);
        std::cout << std::endl;
    }
    
    void Config::write_mode(Mode m)
    {
        std::ofstream ofs;
        ofs.open(config_file_path_, std::ofstream::out | std::ofstream::trunc);
        json_adapter_.write_to_stream(&ofs, json_adapter_.mode_to_json(m));
    }
}  
