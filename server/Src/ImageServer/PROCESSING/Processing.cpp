#include "Processing.h"

#include <Windows.h>

Processing::Processing()
{
	start();
}

Processing::~Processing()
{
	stopAndWait();
}

void Processing::start()
{
	std::lock_guard<std::mutex> guard(m_stop_mutex);
	m_stop_flag = false;
	m_thread = new std::thread(&Processing::run, this);
}

void Processing::stopAndWait()
{
	m_stop_mutex.lock();
	m_stop_flag = true;
	m_stop_mutex.unlock();
	m_thread->join();
}

void Processing::setImage(cv::Mat image)
{
	std::lock_guard<std::mutex> guard(m_imageMutex);
	std::lock_guard<std::mutex> guardOut(m_imageOutputMutex);
	m_image = image;
	if (!m_image.data)
		return;

	m_imageNew = true;
	m_imageOutputReady = false;
}

cv::Mat Processing::imageOutputWait()
{
	while (!isImageOutputReady())
		Sleep(100);

	return m_imageOutput;
}

void Processing::run()
{
	std::cout << "[PROCESSING]" << typeid(*this).name() << "::" << __func__ << " START\n";
	while (!stopFlag())
	{
		if (isImageNew())
		{
			std::cout << " --> "  << "[PROCESSING]" << typeid(*this).name() << "::" << __func__ << " RUNNING\n";
			std::cout << " --> " << "[PROCESSING]" << typeid(*this).name() << m_image.size() << " NEL PROCESSING\n";

			// Altro codice di ispezione
			
			cv::Mat thresholding;
			cv::threshold(m_image, thresholding, 200, 255, cv::THRESH_BINARY);

			cv::Mat elaborateImage = cv::Mat::zeros(m_image.size(), CV_8UC3);
			std::vector<cv::KeyPoint> keypointsVec = getBlobs(thresholding, elaborateImage);
			cv::circle(m_image, { (int)keypointsVec[0].pt.x, (int)keypointsVec[0].pt.y }, keypointsVec[0].size, cv::Scalar(255, 0, 0), cv::FILLED);
			clearImageNew();
			setImageOutput(elaborateImage);
		}
		Sleep(1000);
	}
	std::cout << "[PROCESSING]" << typeid(*this).name() << "::" << __func__ << " END\n";
}

bool Processing::stopFlag()
{
	std::lock_guard<std::mutex>guard(m_stop_mutex);
	return m_stop_flag;
}

bool Processing::isImageNew()
{
	std::lock_guard<std::mutex> guard(m_imageMutex);
	return m_imageNew;
}

void Processing::clearImageNew()
{
	std::lock_guard<std::mutex> guard(m_imageMutex);
	m_imageNew = false;
}

bool Processing::isImageOutputReady()
{
	std::lock_guard<std::mutex> guard(m_imageOutputMutex);
	return m_imageOutputReady;
}

void Processing::setImageOutput(cv::Mat image)
{
	std::lock_guard<std::mutex> guard(m_imageOutputMutex);
	m_imageOutput = image;
	m_imageOutputReady = true;
}

std::vector<cv::KeyPoint> Processing::getBlobs(cv::Mat imageInput, cv::Mat imageOutput)
{
	cv::SimpleBlobDetector::Params params;

	params.filterByColor = true;
	params.blobColor = 255;
	params.filterByArea = false;
	params.minArea = 100;
	params.maxArea = FLT_MAX;
	params.filterByCircularity = false;
	params.filterByConvexity = false;
	params.filterByInertia = false;


	cv::Ptr<cv::SimpleBlobDetector> detector = cv::SimpleBlobDetector::create(params);

	std::vector<cv::KeyPoint> keypoints;
	detector->detect(imageInput, keypoints); // IMAGE BINARIZZATA CON THRESHOLDING 200-255

	
	cv::drawKeypoints(imageInput, keypoints, imageOutput, cv::Scalar(255, 0, 255), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
	if (keypoints.size() <= 0)
	{
		return std::vector<cv::KeyPoint>();
	}
	return keypoints;
}


