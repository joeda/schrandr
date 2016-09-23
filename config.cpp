#include <iostream>
#include <fstream>
#include <sstream>
#include <exception>

#include "config.h"
#include "mode.h"

namespace schrandr {
    
    Config::Config():
        config_file_path_("/tmp/schrandr.conf"),
        empty_setups_path_("/home/jo/dev/schrandr/empty_setups.json")
    {}

    void Config::write_modes(const ModeList &modes)
    {
        std::ofstream ofs;
        ofs.open(config_file_path_, std::ofstream::out | std::ofstream::trunc);
        std::stringstream ss;
        ss.str(std::string());
        Json::Value json_modelist = json_adapter_.modesToJson(modes);
        json_adapter_.write_to_stream(&ss, json_modelist);
        ofs << ss.rdbuf() << std::flush;
    }
    
    ModeList Config::read_modes()
    {
        std::ifstream config_file(config_file_path_);
        if (!config_file.good())
            config_file.open(empty_setups_path_);
        Json::Value json_modes = json_adapter_.read_stream(&config_file);
        
        return json_adapter_.modesFromJson(json_modes);
    }
    
    void Config::print_mode(Mode m)
    {
        json_adapter_.write_to_stream(
            &std::cout, json_adapter_.mode_to_json(m));
    }
    
    void Config::print_modelist(const ModeList &modes)
    {
        json_adapter_.write_to_stream(
            &std::cout ,json_adapter_.modesToJson(modes));
    }
    
    ModeList::ModeList(const ModeList::modeListMap_t &list) {
        modeList_ = list;
    }
    
    bool ModeList::addMode(const MonitorSetup &ms, const Mode &mode) {
        if (modeList_.find(ms)!= modeList_.end()) {
            return false;
        }
        modeList_[ms] = std::map<std::string, Mode>({{"", mode}});
    }
    
    bool ModeList::isMonitorSetupConfigured(const MonitorSetup &ms) const {
        return modeList_.find(ms) != modeList_.end();
    }
    
    std::map<std::string, Mode> ModeList::getModes(const MonitorSetup &ms) const {
        if (modeList_.find(ms) == modeList_.end()) {
            std::map<std::string, Mode> m;
            return m;
        }
        return modeList_.at(ms);
    }
    
    Mode ModeList::getAnyMode(const MonitorSetup &ms) const {
        if (modeList_.find(ms) == modeList_.end()) {
            throw std::runtime_error("MonitorSetup not found");
        }
        for (const auto &modeMap : modeList_.at(ms)) {
            return modeMap.second;
        }
    }
    
    ModeList::modeListMap_t ModeList::getModeList() const {
        return modeList_;
    }
    
    bool ModeList::addNamedMode(const MonitorSetup &ms, const Mode &mode,
            const std::string &name)
    {
        modeList_[ms][name] = mode;
    }
}  
