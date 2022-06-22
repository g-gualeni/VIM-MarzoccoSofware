#pragma once
#include "..\GEOMETRY\GeometricCircle.h"

#include <opencv2/opencv.hpp>

class Checking
{
public:
	void setImage(cv::Mat image);
	
	bool getZeroZone_check();
	bool getFirstZone_check();
private:
	void ZeroZone_checker();
private:
	cv::Mat m_image;
	
	bool m_ZeroZone_check = false;
	bool m_FirstZone_check = false;
};