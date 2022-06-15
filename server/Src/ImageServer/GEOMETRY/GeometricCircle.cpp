#include "GeometricCircle.h"

GeometricCircle::GeometricCircle()
{
}

GeometricCircle::GeometricCircle(const cv::Point& center, int radius)
{
	m_point = center;
	m_radius = radius;
}

cv::Point GeometricCircle::centerAsCvPoint() const
{
	return m_point;
}

int GeometricCircle::radius() const
{
	m_radius;
}
