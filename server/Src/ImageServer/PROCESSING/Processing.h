#pragma once
#include "..\GEOMETRY\GeometricCircle.h"
#include <mutex>
#include <thread>
#include <opencv2/opencv.hpp>
class Processing
{
public:
	Processing();
	~Processing();
	void setImage(cv::Mat image);

	cv::Mat imageOutputWait();
	
private:
	void start();
	void stopAndWait();

	void run();
	bool stopFlag();

	bool isImageNew();
	void clearImageNew();

	bool isImageOutputReady();
	void setImageOutput(cv::Mat image);

	cv::Mat masked(cv::Mat realImage, cv::Mat thresholdImage);
	std::vector<float> findRadiusHolesCountours(cv::Mat imageInput);
	GeometricCircle findFittingCircle(cv::Mat imageInput);

	std::vector<float> radiusConversion(std::vector<float> radiusVector, float conversion);
	void plot_gaussian(std::vector<float> radius, float max, float min);
		

private:
	std::mutex m_stop_mutex;
	bool m_stop_flag;
	std::thread* m_thread = nullptr;

	std::mutex m_imageMutex;
	std::mutex m_imageOutputMutex;

	cv::Mat m_image;
	bool m_imageNew;
	bool m_imageOutputReady = false;
	cv::Mat m_imageOutput;
};