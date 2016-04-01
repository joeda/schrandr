#ifndef SCHRANDR_H_
#define SCHRANDR_H_

#define PID_FILE "/tmp/schrandr.pid"

class PIDFileHolder {
public:
	PIDFileHolder(unsigned int pid);
	~PIDFileHolder();
private:
	std::fstream pid_file_;
};

#endif
