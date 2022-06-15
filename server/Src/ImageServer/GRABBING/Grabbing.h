#pragma once

#include <mutex>
#include <thread>
#include <condition_variable>
#include <opencv2/opencv.hpp>

class Grabbing
{
public:
	Grabbing();
	~Grabbing();

	void start();
	void stopAndWait();

	bool isImageFileMode();
	void setImageFileMode(bool fileMode);
	void loadPath(std::string imageFilePath);

	cv::Mat imageWait(int timeout_ms);
private:
	bool stopFlag();
	void run();

	void loadImagesFromDisk();
	void loadImagesFromCamera();

	
private:
	std::mutex m_stop_mutex;
	bool m_stop_flag = false;
	std::thread* m_thread = nullptr;

	std::mutex m_imageFilePathMutex;
	bool m_imageFileMode = false;
	bool m_imageFilePathNew = false;
	std::condition_variable m_imageFilePathReady;
	std::condition_variable m_imageReady;

	std::string m_imageFilePath;

	std::mutex m_imageMutex;
	bool m_imageEmpty = false;

	cv::Mat m_image;

	

};