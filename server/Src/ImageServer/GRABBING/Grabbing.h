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


	bool isImageFileMode();
	void setImageFileMode(bool fileMode);
	void loadPath(std::string imageFilePath);

	cv::Mat imageWait(int timeout_ms);

	void setImageEmpty(bool imageMode);
	void setStopGrabbing(bool stop);
private:
	void start();
	void stopAndWait();
	bool stopFlag();
	void run();

	void loadImagesFromDisk();
	void loadImagesFromCamera();

	bool isImageEmpty();
	bool isStopped();

	
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

	std::mutex m_stop_grabbing;
	bool m_stopGrabbing = false;
	bool m_printMessage = false;
	

	

};