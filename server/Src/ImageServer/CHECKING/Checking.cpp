#include <Windows.h>

#include "Checking.h"
#include "..\UTILITY\ElapsedTime.h"



Checking::Checking()
{
}

Checking::~Checking()
{
}

void Checking::setImage(cv::Mat image)
{
	m_image = image;
}

bool Checking::getZeroZone_check()
{
	ZeroZone_checker();
	return m_ZeroZone_check;
}

bool Checking::getBoolTimer()
{
	return m_timer;
}

bool Checking::getFirstZone_check()
{
	return m_FirstZone_check;
}

void Checking::ZeroZone_checker()
{
	ElapsedTime ET;
	ET.start();
	if (m_image.empty())
		return;

	cv::Mat workedImage;
	cv::cvtColor(m_image, workedImage, cv::COLOR_BGR2GRAY);

	cv::Mat thresholding;
	cv::threshold(workedImage, thresholding, 200, 255, cv::THRESH_BINARY);

	std::vector<std::vector<cv::Point> > contours;
	std::vector<cv::Vec4i> hierarchy;
	cv::findContours(thresholding, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE, cv::Point(0, 0));

	std::vector<std::vector<cv::Point> > outerContours;
	for (size_t i = 0; i < contours.size(); i++)
	{
		if (contours[i].size() > 20) // PARAMETRO 20 DA TENERE FISSO
			outerContours.push_back(contours.at(i));
	}
	std::vector<std::vector<cv::Point> > contours_poly(outerContours.size());
	std::vector<cv::Point2f>center(outerContours.size());
	std::vector<float>radius(outerContours.size());
	std::vector<GeometricCircle> circles;

	cv::Mat drawingContours = cv::Mat::zeros(thresholding.size(), CV_8UC3);
	cv::Mat drawingCircles = cv::Mat::zeros(thresholding.size(), CV_8UC3);
	cv::Mat1b contour_mask(thresholding.rows, thresholding.cols, uchar(0));
	for (size_t i = 0; i < outerContours.size(); i++)
	{
		approxPolyDP(cv::Mat(outerContours[i]), contours_poly[i], 3, false);
		cv::minEnclosingCircle((cv::Mat)contours_poly[i], center[i], radius[i]);
		drawContours(drawingContours, contours_poly, i, cv::Scalar(255, 0, 255), 5, 8, std::vector<cv::Vec4i>(), 0, cv::Point());
		cv::drawContours(contour_mask, contours_poly, i, cv::Scalar(255), cv::FILLED);
		circle(drawingCircles, center[i], radius[i], cv::Scalar(0, 255, 255), 5, 8, 0);
		if (!center.empty())
			circles.push_back(GeometricCircle(center[i], radius[i]));
	}

	if (circles.empty()) //Non ci sono circonferenze
		return;			

	bool trovato = false;
	int biggest = 0;
	for (size_t i = 0; i < circles.size(); i++)
	{
		if (circles[i].radius() > 850) // PARAMETRO 850 DA TENERE FISSO 
		{
			trovato = true;
			biggest = i;
			break;
		}

	}

	cv::Mat1b circle_mask(thresholding.rows, thresholding.cols, uchar(0));
	if (!trovato) // NON E' stato individuato il cerchio esterno 
		return;
	else
		circle(circle_mask, center[biggest], radius[biggest], cv::Scalar(255), cv::FILLED);


	int whitePixel_inside_countor = cv::countNonZero(contour_mask);
	int whitePixel_inside_circle = cv::countNonZero(circle_mask);
	int difference = abs(whitePixel_inside_circle - whitePixel_inside_countor);
	double theoryTollerance = (double)difference / std::min(whitePixel_inside_circle, whitePixel_inside_countor) * 100;
	double realTollerance = 0.05 * 100; // PARAMETRO FISSO, TOLLERANZA DEL 5%

	std::cout << "PIXEL BIANCHI CONTORNO: " << whitePixel_inside_countor << "\n";
	std::cout << "PIXEL BIANCHI CERCHIO: " << whitePixel_inside_circle << "\n";
	std::cout << "TOLLERANZA: " << theoryTollerance << "\n";

	if (theoryTollerance <= realTollerance)
		m_ZeroZone_check = true; // Immagine senza ingombro
	else
		m_timer = true;

	std::cout << ET.elapsed("[CHECKING]: checker ZeroZONE ") << "\n";
}


/*
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
*/
