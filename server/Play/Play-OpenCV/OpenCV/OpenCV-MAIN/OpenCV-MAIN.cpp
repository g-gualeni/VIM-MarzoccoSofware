
#include <opencv2/opencv.hpp>

#include <iostream>
#include <vector>

int contours(cv::Mat image)
{
	std::vector<std::vector<cv::Point> > contours;
	std::vector<cv::Vec4i> hierarchy;
	cv::findContours(image, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE, cv::Point(0, 0));
	std::vector<std::vector<cv::Point> > contours_poly(contours.size());
	std::vector<cv::Point2f>center(contours.size());
	std::vector<float>radius(contours.size());

	

	int total_white_inside_contour = 0;
	int white_on_image_inside_contour = 0;
	int black_on_image_inside_contour = 0;

	cv::Mat1b contour_mask(image.rows, image.cols, uchar(0));
	for (int i = 0; i < contours.size(); i++)
	{
		approxPolyDP(cv::Mat(contours[i]), contours_poly[i], 3, false);
		cv::minEnclosingCircle((cv::Mat)contours_poly[i], center[i], radius[i]);

		if(contours[i].size() > 50)
			cv::drawContours(contour_mask, contours, i, cv::Scalar(255), 8);

		total_white_inside_contour = cv::countNonZero(contour_mask);
		white_on_image_inside_contour = cv::countNonZero(image & contour_mask);
		black_on_image_inside_contour = total_white_inside_contour - white_on_image_inside_contour;
	}
		

	/// Draw polygonal contour + bonding rects + circles
	cv::Mat drawingContoursApproximately = cv::Mat::zeros(image.size(), CV_8UC3);
	for (int i = 0; i < contours.size(); i++)
	{
		
		drawContours(drawingContoursApproximately, contours_poly, i, cv::Scalar(255, 0, 255), 5, 8, std::vector<cv::Vec4i>(), 0, cv::Point());
		
		/*
		if((int)radius[i] < 50)
			circle(drawing, center[i], (int)radius[i], cv::Scalar(255, 0, 255), cv::FILLED, 8, 0);
		else
			circle(drawing, center[i], (int)radius[i], cv::Scalar(0, 255, 255), 5, 8, 0);
			*/
	}
	
	
	std::cout << "PIXEL BIANCHI: " << white_on_image_inside_contour << "\n";
	std::cout << "PIXEL NERI: " << black_on_image_inside_contour << "\n";
}

int blobCircleOuter(cv::Mat image)
{
	cv::SimpleBlobDetector::Params params;

	params.filterByColor = false;
	
	params.filterByArea = true;
	params.minArea = 100;
	params.filterByCircularity = false;
	params.minCircularity = 0.9f;
	params.maxCircularity = std::numeric_limits<float>::max();
	
	params.filterByConvexity = false;
	
	params.filterByInertia = false;
	
	


	cv::Ptr<cv::SimpleBlobDetector> detector = cv::SimpleBlobDetector::create(params);

	std::vector<cv::KeyPoint> keypoints;
	detector->detect(image, keypoints); // IMAGE BINARIZZATA CON THRESHOLDING 200-255

	cv::Mat imageDrawing;
	cv::drawKeypoints(image, keypoints, imageDrawing, cv::Scalar(255, 0, 255), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
	
	/*
	for (int i = 0; i < keypoints.size(); i++)
		circle(imageDrawing, cv::Point(keypoints[i].pt.x, keypoints[i].pt.y), keypoints[i].size, cv::Scalar(255, 0, 255), cv::FILLED, 8, 0);
	*/	

	if (keypoints.size() <= 0)
	{
		return -1;
	}
	return 0;
}
int main()
{
    cv::Mat m_image = cv::imread("C:\\GitHub\\VIM-MarzoccoSofware\\server\\Res\\manoOutside.tiff");
	cv::Mat m_image1c;
	cv::cvtColor(m_image, m_image1c, cv::COLOR_BGR2GRAY);
	
	cv::Mat thresholding;
	cv::threshold(m_image, thresholding, 200, 255, cv::THRESH_BINARY);
	cv::Mat thresholding1c;
	cv::threshold(m_image1c, thresholding1c, 200, 255, cv::THRESH_BINARY);
	contours(thresholding1c);
	blobCircleOuter(thresholding);
}


