#include <iostream>
#include <string>
#include <fstream>

// Required by for routine
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>                                       // flock
#include <unistd.h>

#include <stdlib.h>

bool file_exists(const std::string& name)
{
    struct stat buffer;  
    return (stat (name.c_str(), &buffer) == 0);
}

int main(void)
{
    std::string pid_file = "/tmp/schrandr.pid";
    unsigned int loop_duration = 500000;
    
    pid_t pID = fork();
    if (pID == 0)                                           // child
    {
        // Code only executed by child process
        std::cout << "I'm the child process!" << std::endl;
        setsid();
        if (file_exists(pid_file)) {
            std::cout << "PIDFILE found!" << std::endl;
        } else {
            std::cout << "PIDFILE not found!" << std::endl;
        }

        std::fstream pid_file_stream;
        pid_file_stream.open(pid_file.c_str(), std::fstream::in | std::fstream::out |std::fstream::app);
        std::cout << "Do I even get here?" << std::endl;
        if ( (pid_file_stream.rdstate() & std::ifstream::failbit ) != 0 ) {
            std::cout << "PIDFile could not be opened." << std::endl;
            exit(EXIT_FAILURE);
        } else {
            std::cout << "PIDFile opened." << std::endl;
        }
        while (true) {
            std::cout << "Infinite Loop!" << std::endl;
            usleep(loop_duration);
        }
    }
    else if (pID < 0)                                       // failed to fork
    {
        std::cerr << "Failed to fork" << std::endl;
        exit(EXIT_FAILURE);
    // Throw exception
    }
    else                                                    // parent
    {
        // Code only executed by parent process
        std::cout << "I'm the parent process! Goodbye!" << std::endl;  
    }

}
