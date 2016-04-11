#include <syslog.h>
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
        if (syslog_) closelog();
    }
    
    
    void Logger::enable_syslog()
    {
        openlog("schrandr", LOG_CONS, LOG_USER);
        syslog_ = true;
    }
    
    void Logger::log(std::string msg) 
    {
        syslog(LOG_INFO, msg.c_str());
    }
    
    void Logger::log(char* msg)
    {
        syslog(LOG_INFO, msg);
    }
}
