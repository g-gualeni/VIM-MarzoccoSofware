#include "ElapsedTime.h"

ElapsedTime::ElapsedTime()
{
	m_start = std::chrono::high_resolution_clock::now();
}

void ElapsedTime::start()
{
	std::lock_guard<std::mutex> lock(m_start_mutex);
	m_start = std::chrono::high_resolution_clock::now();
}

long long ElapsedTime::elapsed_us()
{
	auto now = std::chrono::high_resolution_clock::now();
	std::lock_guard<std::mutex> lock(m_start_mutex);
	return std::chrono::duration_cast<std::chrono::microseconds>(now - m_start).count();
}

std::string ElapsedTime::elapsed(const char * msg)
{
	auto val = std::string(msg);
	return elapsed(val);
}

std::string ElapsedTime::elapsed(std::string & msg)
{
	auto now = std::chrono::high_resolution_clock::now();
	std::lock_guard<std::mutex> lock(m_start_mutex);
	auto delta_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(now - m_start).count();
	return msg + format_ns(delta_ns);
}

std::string ElapsedTime::format_ns(long long delta_ns)
{
	const int buffSize = 100;
	char buffer[buffSize];

	if (delta_ns < 1000) {
		int res = snprintf(buffer, buffSize, "%d [ns]", long(delta_ns));
	}
	else if (delta_ns < 100 * 1000) {
		int res = snprintf(buffer, buffSize, "%lld - %0.2f [us]", delta_ns, double(delta_ns) / 1000.0);
	}
	else if (delta_ns < 1000 * 1000) {
		int res = snprintf(buffer, buffSize, "%lld - %0.2f [ms]", delta_ns, (double(delta_ns) / (1000.0 * 1000)));
	}
	else if (delta_ns < 100LL * 1000 * 1000) {
		int res = snprintf(buffer, buffSize, "%lld - %0.2f [ms]", delta_ns, (double(delta_ns) / (1000.0 * 1000)));
	}
	else if (delta_ns < 1000LL * 1000 * 1000) {
		int res = snprintf(buffer, buffSize, "%lld - %0.2f [s]", delta_ns, (double(delta_ns) / (1000.0 * 1000 * 1000)));
	}
	else {
		int res = snprintf(buffer, buffSize, "%lld - %0.2f [s]", delta_ns, (double(delta_ns) / (1000.0 * 1000 * 1000)));
	}

	return std::string(buffer);
}
