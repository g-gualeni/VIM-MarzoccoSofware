#pragma once
#include <opencv2/opencv.hpp>
class Grab_ChunkImage_Universal
{
public:
	static cv::Mat imageFromCamera(cv::Mat grabImage);
};
