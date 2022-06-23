
#include <opencv2/opencv.hpp>

#include <iostream>
#include <vector>

class GeometricCircle
{
public:
	GeometricCircle(const cv::Point& center, int radius);
	int radius();
private:
	float m_radius;
	cv::Point m_point;
};

GeometricCircle::GeometricCircle(const cv::Point& center, int radius)
{
	m_radius = radius;
	m_point = center;
}
int GeometricCircle::radius() 
{
	return m_radius;
}
int contours(cv::Mat image)
{
	bool imageZ0 = false;
	std::vector<std::vector<cv::Point> > contours;
	std::vector<cv::Vec4i> hierarchy;
	cv::findContours(image, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE, cv::Point(0, 0));

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

	cv::Mat drawingContours = cv::Mat::zeros(image.size(), CV_8UC3);
	cv::Mat drawingCircles = cv::Mat::zeros(image.size(), CV_8UC3);
	cv::Mat1b contour_mask(image.rows, image.cols, uchar(0));
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
		return -1; // RETURN FALSE

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

	cv::Mat1b circle_mask(image.rows, image.cols, uchar(0));
	if (!trovato) // NON E' stato individuato il cerchio esterno 
		return -1;
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
		imageZ0 = true; // Immagine senza ingombro
	
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
	
	
	for (int i = 0; i < keypoints.size(); i++)
		circle(imageDrawing, cv::Point(keypoints[i].pt.x, keypoints[i].pt.y), keypoints[i].size, cv::Scalar(255, 0, 255), cv::FILLED, 8, 0);
	

	if (keypoints.size() <= 0)
	{
		return -1;
	}
	return 0;
}
int main()
{
    cv::Mat m_image = cv::imread("C:\\GitHub\\VIM-MarzoccoSofware\\server\\Res\\Con_filtro.tiff");
	cv::Mat m_image1c;
	cv::cvtColor(m_image, m_image1c, cv::COLOR_BGR2GRAY);
	
	cv::Mat thresholding;
	cv::threshold(m_image, thresholding, 200, 255, cv::THRESH_BINARY);
	cv::Mat thresholding1c;
	cv::threshold(m_image1c, thresholding1c, 200, 255, cv::THRESH_BINARY);
	contours(thresholding1c);
	blobCircleOuter(thresholding);
}

