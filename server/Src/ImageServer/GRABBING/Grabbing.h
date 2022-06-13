#pragma once

#include <mutex>
#include <thread>
class Grabbing
{
public:
	Grabbing();
	~Grabbing();

	void start();
	void stopAndWait();

	bool isImageFileMode();
	void setImageFileMode(bool fileMode);
private:
	bool stopFlag();
	void run();
private:
	std::mutex m_stop_mutex;
	bool m_stop_flag = false;
	std::thread* m_thread = nullptr;

	std::mutex m_imageFilePathMutex;
	bool m_imageFileMode = false;

	std::mutex m_imageMutex;
	bool m_imageEmpty = false;
};