#ifndef SCHRANDR_H_
#define SCHRANDR_H_

#include <iostream>
#include <fstream>

namespace schrandr {

class PIDFileHolder {
public:
    PIDFileHolder(unsigned int pid,
                  const std::string pid_file = std::string("/tmp/schrandr.pid"));
    ~PIDFileHolder();
private:
    std::fstream pid_file_;
    std::string pid_file_path_;
};
}

#endif
