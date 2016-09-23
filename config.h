#ifndef CONFIG_H_
#define CONFIG_H_

#include <string>
#include <json/json.h>
#include <vector>

#include "mode.h"
#include "jsonadapter.h"

namespace schrandr {
        
    class ModeList {
    public:
        typedef std::map<MonitorSetup, std::map<std::string, Mode> > modeListMap_t;
        
        ModeList() = default;
        ModeList(const modeListMap_t &list);
        
        bool addMode(const MonitorSetup &ms, const Mode &mode);
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
