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
			
			clearImageNew();
			// OUTPUT PDF, NOT IMAGE
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

std::vector<GeometricCircle> Processing::findCountours(cv::Mat imageThreshold)
{
	std::vector<std::vector<cv::Point> > contours;
	std::vector<cv::Vec4i> hierarchy;
	cv::findContours(imageThreshold, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE, cv::Point(0, 0));

	std::vector<std::vector<cv::Point> > holesContours;
	std::vector<std::vector<cv::Point> > outerContours;
	for (size_t i = 0; i < contours.size(); i++)
	{
		if (contours[i].size() > 20) // PARAMETRO 20 DA TENERE FISSO
			outerContours.push_back(contours.at(i));
	}

	double dim = DBL_MAX;
	int minIndex = 0;
	
	for (size_t i = 0; i < outerContours.size(); i++)
	{
		if (dim < outerContours[i].size())
			minIndex = i;

	}
	for (size_t i = 0; i < contours.size(); i++)
	{
		if (contours[i].size() < 20) // PARAMETRO 20 DA TENERE FISSO
			holesContours.push_back(contours.at(i));
	}
	std::vector<std::vector<cv::Point> > contours_poly(holesContours.size());
	std::vector<cv::Point2f>center(holesContours.size());
	std::vector<float>radius(holesContours.size());
	std::vector<GeometricCircle> circles;

	cv::Mat drawingContours = cv::Mat::zeros(imageThreshold.size(), CV_8UC3);
	cv::Mat drawingCircles = cv::Mat::zeros(imageThreshold.size(), CV_8UC3);
	cv::Mat1b contour_mask(imageThreshold.rows, imageThreshold.cols, uchar(0));
	for (size_t i = 0; i < holesContours.size(); i++)
	{
		approxPolyDP(cv::Mat(holesContours[i]), contours_poly[i], 3, false);
		cv::minEnclosingCircle((cv::Mat)contours_poly[i], center[i], radius[i]);
		drawContours(drawingContours, contours_poly, i, cv::Scalar(255, 0, 255), 5, 8, std::vector<cv::Vec4i>(), 0, cv::Point());
		drawContours(contour_mask, contours_poly, i, cv::Scalar(255), cv::FILLED);
		circle(drawingCircles, center[i], radius[i], cv::Scalar(0, 255, 255), 5, 8, 0);
		if (!center.empty())
			circles.push_back(GeometricCircle(center[i], radius[i]));
	}
	return circles;
}




