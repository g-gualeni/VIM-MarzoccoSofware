#pragma once
#include "..\GEOMETRY\GeometricCircle.h"

#include <opencv2/opencv.hpp>
#include <mutex>

class Checking
{
public:
	Checking();
	~Checking();
	void setImage(cv::Mat image);
	
	bool getZeroZone_check();
	bool getBoolTimer();
	bool getFirstZone_check();

	void ZonesChecker();

	void setZeroZone_check(bool zeroZone);
	void setFirstZone_check(bool firstZone);
	
private:
private:
	cv::Mat m_image;
		
	bool m_ZeroZone_check = false;

	std::mutex m_mutexTimer;
	bool m_timer = false;

	bool m_FirstZone_check = false;

	
};