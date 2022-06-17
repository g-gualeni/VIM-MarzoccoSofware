#pragma once
#include "..\GEOMETRY\GeometricCircle.h"
#include <mutex>
#include <thread>
#include <opencv2/opencv.hpp>


class Checking
{
public:
	Checking();
	~Checking();

	void start();
	void stopAndWait();

	void setImage(cv::Mat image);
	cv::Mat imageWait(int timeout_ms);

private:
	void run();
	bool isImageOutputReady();
	void setImageOutput(cv::Mat imageOutput);
	void clearImageNew();
	bool stopFlag();
	bool isImageNew();
	


	int blobCircleOuter(cv::Mat image);

private:
	cv::Mat m_image;

	std::mutex m_stop_mutex;
	bool m_stop_flag = false;
	std::thread* m_thread = nullptr;

	std::mutex m_imageMutex;
	bool m_imageNew = false;
	std::mutex m_imageOutputMutex;
	cv::Mat m_imageOutput;
	bool m_imageOutputReady = false;


};