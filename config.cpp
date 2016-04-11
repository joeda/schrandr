#include <iostream>
#include <fstream>

#include "config.h"
#include "monitor_setup.h"

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
        Json::StyledStreamWriter writer;
        writer.write(ofs, setups_as_json_);
    }
    
    Json::Value Config::setup_to_json_(MonitorSetup ms)
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
    
    void Config::add_setup(MonitorSetup ms)
    {
        known_setups_.push_back(ms);
    }
    
    bool Config::has_setup(MonitorSetup ms)
    {
        for(auto const& monitor_entry: known_setups_) {
            if (monitor_entry == ms)
                return true;
        }
        return false;
    }
    
    void Config::read()
    {
        std::ifstream config_file(config_file_path_);
        if (!config_file.good())
            config_file.open(empty_setups_path_);
        Json::Reader reader;
        bool parsed_success = reader.parse(config_file, setups_as_json_, false);
    }
    
    void Config::print_all() 
    {
        Json::StyledWriter writer;
        std::string stuff = writer.write(setups_as_json_);
        std::cout << stuff << std::endl;
    }
    std::string Config::to_string()const
    {
        
    }
} 
