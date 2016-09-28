#ifndef CONFIG_H_
#define CONFIG_H_

#include <string>
#include <json/json.h>
#include <vector>
#include <memory> //shared_ptr

#include "mode.h"
#include "jsonadapter.h"
#include "logging.h"

namespace schrandr {
            
    class ModeList {
    public:
        typedef std::map<MonitorSetup, std::map<std::string, Mode> > modeListMap_t;
        
        ModeList() = default;
        ModeList(const modeListMap_t &list);
        
        bool addMode(const MonitorSetup &ms, const Mode &mode);
        void setDefaultMode(const MonitorSetup &ms, const Mode &mode);
        bool addNamedMode(const MonitorSetup &ms, const Mode &mode,
            const std::string &name);
        bool isMonitorSetupConfigured(const MonitorSetup &ms) const;
        std::map<std::string, Mode> getModes(const MonitorSetup &ms) const;
        Mode getAnyMode(const MonitorSetup &ms) const;
        modeListMap_t getModeList() const;
        
    private:
         modeListMap_t modeList_;
    };
    
    class Config {
    public:
        Config(const std::string &configDir,
               const std::string &confFileName,
               std::shared_ptr<Logger> logger);
        void print_mode(Mode m);
        void print_modelist();
        void handleModeChange(const MonitorSetup &ms, const Mode &mode);
        Mode getAnyMode(const MonitorSetup &ms) const;
        bool isMonitorSetupConfigured(const MonitorSetup &ms) const;
        
    private:
        std::string confFileName_;
        std::string empty_setups_path_;
        std::string configDir_;
        JSONAdapter json_adapter_;
        ModeList modeList_;
        std::shared_ptr<Logger> logger_;
        
        void writeModes_();
        void readModes_();
    };

} 

#endif 
