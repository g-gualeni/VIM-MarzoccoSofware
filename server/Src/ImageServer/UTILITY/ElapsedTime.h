#pragma once

#include <chrono>
#include <mutex>
#include <string>

/*
	Time measurement support class
	 - Start the measurement creating the class
	 - Mutex protect access to be multithread safe
*/

class ElapsedTime
{
public:
	ElapsedTime();
	void start();
	long long elapsed_us();
	std::string elapsed(const char* msg);
	std::string elapsed(std::string &msg);
	static std::string format_ns(long long delta_ns);
	
private:
	std::mutex m_start_mutex;
	std::chrono::steady_clock::time_point m_start;

};

