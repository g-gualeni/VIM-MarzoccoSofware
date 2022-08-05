#pragma once
#include <opencv2/opencv.hpp>

class GeometricCircle
{
public:
	GeometricCircle();
	GeometricCircle(const cv::Point& center, float radius);
	cv::Point centerAsCvPoint() const;
	int radius() const;
private:
	float m_radius;
	cv::Point m_point;
};