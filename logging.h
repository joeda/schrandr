#ifndef LOGGING_H_
#define LOGGING_H_

#include <syslog.h>


namespace schrandr {
    
    class Logger {
    public:
        Logger();
        ~Logger();
        void enable_syslog();
        void log(const std::string &msg);
        void log(char *msg);
        void log(int level, const std::string &msg);
        void log(int level, char* msg);
        
        template<class T>
        void log(std::vector<T> messages)
        {
            std::cout << "Debug 2" << std::endl;
            if (!messages.empty()) {
                for(typename std::vector<T>::iterator it = messages.begin();
                    it != messages.end();
                    ++it) {
                    log(*it);
                }
            }
        }
    private:
        bool syslog_;
    };
} 

#endif
