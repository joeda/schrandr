#include <iostream>
#include <fstream>
#include <sstream>
#include <exception>

#include "config.h"
#include "mode.h"

namespace schrandr {
    
    Config::Config(const std::string &configDir,
                   const std::string &confFileName,
                   std::shared_ptr<Logger> logger)
    :
        configDir_(configDir),
        confFileName_(confFileName),
        empty_setups_path_("/home/jo/dev/schrandr/empty_setups.json"),
        logger_(logger)
    {
        readModes_();
    }

    void Config::writeModes_()
    {
        std::ofstream ofs;
        ofs.open(configDir_ + std::string("/") + confFileName_,
                 std::ofstream::out | std::ofstream::trunc);
        std::stringstream ss;
        ss.str(std::string());
        Json::Value json_modelist = json_adapter_.modesToJson(modeList_);
        json_adapter_.write_to_stream(&ss, json_modelist);
        ofs << ss.rdbuf() << std::flush;
        ofs.close();
    }
    
    void Config::readModes_()
    {
        auto filePath = configDir_ + std::string("/") + confFileName_;
        std::ifstream config_file(filePath);
        if (!config_file.good()) {
            logger_->log(LOG_NOTICE, "Config file " + filePath + " not found, "
                "reverting to " + empty_setups_path_);
            filePath = empty_setups_path_;
            config_file.open(empty_setups_path_);
        }
        Json::Value json_modes = json_adapter_.read_stream(&config_file);
        
        modeList_ = json_adapter_.modesFromJson(json_modes);
        config_file.close();
        logger_->log(LOG_INFO, "Successfully read " + empty_setups_path_);
    }
    
    void Config::print_mode(Mode m)
    {
        json_adapter_.write_to_stream(
            &std::cout, json_adapter_.mode_to_json(m));
    }
    
    void Config::print_modelist()
    {
        json_adapter_.write_to_stream(
            &std::cout ,json_adapter_.modesToJson(modeList_));
    }
    void Config::handleModeChange(const MonitorSetup &ms, const Mode &mode)
    {
        modeList_.setDefaultMode(ms, mode);
        writeModes_();
    }
    
    Mode Config::getAnyMode(const MonitorSetup &ms) const
    {
        return modeList_.getAnyMode(ms);
    }
    
    bool Config::isMonitorSetupConfigured(const MonitorSetup &ms) const
    {
        return modeList_.isMonitorSetupConfigured(ms);
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
    
    void ModeList::setDefaultMode(const MonitorSetup &ms, const Mode &mode) {
        modeList_[ms][""] = mode;
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
