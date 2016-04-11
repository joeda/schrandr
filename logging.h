#ifndef LOGGING_H_
#define LOGGING_H_


namespace schrandr {
    
    class Logger {
    public:
        Logger();
        ~Logger();
        void enable_syslog();
        void log(std::string msg);
        void log(char *msg);
        
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
