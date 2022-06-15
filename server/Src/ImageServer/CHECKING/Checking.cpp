#include "Checking.h"
#include <iostream>
#include <windows.h>


Checking::Checking()
{
	start();
}

Checking::~Checking()
{
	stopAndWait();
}

void Checking::start()
{
	std::lock_guard<std::mutex> guard(m_stop_mutex);
	m_stop_flag = false;
	m_thread = new std::thread(&Checking::run, this);
}

void Checking::stopAndWait()
{
	m_stop_mutex.lock();
	m_stop_flag = true;
	m_stop_mutex.unlock();
	m_thread->join();
	std::cout << typeid(*this).name() << "::" << __func__ << " END\n";
}

void Checking::setImage(cv::Mat image)
{
	std::lock_guard<std::mutex> guard(m_imageMutex);
	std::lock_guard<std::mutex> guardOut(m_imageOutputMutex);
	m_image = image;
	if (!m_image.data)
		return;

	m_imageNew = true;
	m_imageOutputReady = false;
}

cv::Mat Checking::imageWait(int timeout_ms)
{
	while (!isImageOutputReady())
	{
		Sleep(100);
	}
	return m_imageOutput;
	
}

void Checking::run()
{
	while (!stopFlag())
	{
		if (isImageNew())
		{
			blobCircleOuter(m_image);
		}
		else
		{
			Sleep(1000);
		}
	}
	// SETTARE LA NUOVA IMMAGINE
	clearImageNew();
}

bool Checking::isImageOutputReady()
{
	std::lock_guard<std::mutex> guard(m_imageOutputMutex);
	return m_imageOutputReady;
}

void Checking::setImageOutput(cv::Mat imageOutput)
{
	std::lock_guard<std::mutex> guard(m_imageOutputMutex);
	m_imageOutput = imageOutput;
	m_imageOutputReady = true;
}

void Checking::clearImageNew()
{
	std::lock_guard<std::mutex> guard(m_imageMutex);
	m_imageNew = false;
}

bool Checking::stopFlag()
{
	std::lock_guard<std::mutex> guard(m_stop_mutex);
	return m_stop_flag;
}

bool Checking::isImageNew()
{
	std::lock_guard<std::mutex> guard(m_imageMutex);
	return m_imageNew;
}

int Checking::blobCircleOuter(cv::Mat image)
{
	cv::Mat BGRimage;
	cv::cvtColor(image, BGRimage, cv::COLOR_GRAY2BGR);
	cv::Mat thresholding;
	cv::bitwise_not(BGRimage, thresholding);
	cv::SimpleBlobDetector::Params params;
	params.minThreshold = 10;
	params.maxThreshold = 200;
	params.filterByArea = true;
	params.minArea = 1000;	
	params.maxArea = FLT_MAX;
	params.filterByCircularity = true;
	params.minCircularity = 0.5;
	params.filterByConvexity = true;
	params.minConvexity = 0.2;
	params.filterByInertia = true;
	params.minInertiaRatio = 0.7;

	cv::Ptr<cv::SimpleBlobDetector> detector = cv::SimpleBlobDetector::create(params);

	std::vector<cv::KeyPoint> keypoints;
	detector->detect(thresholding, keypoints);
	cv::Mat img;
	cv::drawKeypoints(BGRimage, keypoints, BGRimage, cv::Scalar(255, 0, 255), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
	if (keypoints.size() <= 0)
	{
		return -1;
	}
	return 0;
}
