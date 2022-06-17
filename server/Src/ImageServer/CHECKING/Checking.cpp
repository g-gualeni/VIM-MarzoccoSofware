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
			cv::Mat BGRimage;
			cv::cvtColor(m_image, BGRimage, cv::COLOR_GRAY2BGR);

			cv::Mat thresholding;
			cv::threshold(BGRimage, thresholding, 200, 255, cv::THRESH_BINARY);
			
			
			blobCircleOuter(thresholding);
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
	cv::SimpleBlobDetector::Params params;
	
	params.minThreshold = 10;
	params.maxThreshold = 200;
	params.filterByColor = true;
	params.blobColor = 255;
	params.filterByArea = true;
	params.minArea = 100;
	params.maxArea = FLT_MAX;
	params.filterByCircularity = false;
	params.filterByConvexity = false;
	params.filterByInertia = false;
	

	cv::Ptr<cv::SimpleBlobDetector> detector = cv::SimpleBlobDetector::create(params);

	std::vector<cv::KeyPoint> keypoints;
 	detector->detect(image, keypoints); // IMAGE BINARIZZATA CON THRESHOLDING 200-255
	
	cv::Mat imageDrawing;
	cv::drawKeypoints(image, keypoints, imageDrawing, cv::Scalar(255, 0, 255), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
	if (keypoints.size() <= 0)
	{
		return -1;
	}
	return 0;
}
