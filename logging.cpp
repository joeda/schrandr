#include <iostream>
#include <memory>
#include <vector>

#include "logging.h"


namespace schrandr {
        
    Logger::Logger():
        syslog_(false)
        {}

    Logger::~Logger()
    {
        log("Closing log and exiting");
        if (syslog_) closelog();
    }
    
    
    void Logger::enable_syslog()
    {
        openlog("schrandr", LOG_CONS, LOG_USER);
        syslog_ = true;
    }
    
    void Logger::log(const std::string &msg) 
    {
        syslog(LOG_INFO, msg.c_str());
    }
    
    void Logger::log(char* msg)
    {
        syslog(LOG_INFO, msg);
    }
    
    void Logger::log(int level, const std::string &msg)
    {
        syslog(level, msg.c_str());
    }
    
    void Logger::log(int level, char* msg)
    {
        syslog(level, msg);
    }
}
