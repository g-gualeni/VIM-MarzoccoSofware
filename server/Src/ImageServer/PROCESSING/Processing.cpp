#include "Processing.h"
#include "..\UTILITY\Math.h"
#include "..\UTILITY\\matplotlibcpp.h"
#include "boost/math/statistics/univariate_statistics.hpp"
#include "..\UTILITY\Aspose.Pdf.h"
#include <Windows.h>
#include <cmath>

namespace plt = matplotlibcpp;
namespace bst = boost::math::statistics;


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
			
			cv::Mat thresholdingImage;
			cv::threshold(m_image, thresholdingImage, 200, 255, cv::THRESH_BINARY);

			cv::Mat thresholdingMask = masked(m_image, thresholdingImage);
			cv::threshold(thresholdingMask, thresholdingMask, 110, 255, cv::THRESH_BINARY);

			GeometricCircle fittingCircle = findFittingCircle(thresholdingImage);
			std::vector<float> radiusHoles = findRadiusHolesCountours(thresholdingMask);
			
			float mm_conversion = 35.0 / fittingCircle.radius();  // 35 E' il raggio del filtro 21g 
			std::vector<float> diamHolesConverted = radiusConversion(radiusHoles, mm_conversion);

			float maxHoleDiam = Math::takeMax(diamHolesConverted);
			float minHoleDiam = Math::takeMin(diamHolesConverted);
			int indexMaxDiam = Math::index(maxHoleDiam, diamHolesConverted);
			int indexMinDiam = Math::index(minHoleDiam, diamHolesConverted);

			std::cout << "MM CONVERSION: " << mm_conversion << "\n";
			std::cout << "DIAMETRO MINIMO: " << minHoleDiam << "\n";
			std::cout << "DIAMETRO MASSIMO: " << maxHoleDiam << "\n";
			std::cout << "INDICE DIAMETRO MINIMO: " << indexMinDiam << "\n";
			std::cout << "INDICE DIAMETRO MASSIMO: " << indexMaxDiam << "\n";
												
			plot_gaussian(diamHolesConverted, maxHoleDiam, minHoleDiam);
			// OUTPUT PDF, NOT IMAGE

			// Create document
			auto doc = System::MakeObject<Aspose::Pdf::Document>();
			auto pages = doc->get_Pages();
			pages->Add();
			auto page = pages->idx_get(1);// Numbering of pages starts from index 1.
			auto paragraps = page->get_Paragraphs();
			auto text = System::MakeObject<Aspose::Pdf::Text::TextFragment>(u"Hello World");
			auto ts = text->get_TextState();
			ts->set_FontSize(16);
			ts->set_FontStyle(Aspose::Pdf::Text::FontStyles::Italic);
			paragraps->Add(text);
			paragraps->Add(System::MakeObject<Aspose::Pdf::Text::TextFragment>(u"This example shows how to create a PDF with text using Aspose.PDF for C++."));
			doc->Save(u"Example1.pdf");			
			
			clearImageNew();
			setImageOutput(thresholdingMask);
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

cv::Mat Processing::masked(cv::Mat realImage, cv::Mat imageThreshold)
{
	GeometricCircle fittingCircle = findFittingCircle(imageThreshold);
	
	// CREATE A MASK 
	cv::Mat mask(realImage.size(), realImage.type());
	mask.setTo(0);
	cv::Point center_mask(fittingCircle.centerAsCvPoint().x, fittingCircle.centerAsCvPoint().y);
	const int radius_mask = fittingCircle.radius();
	circle(mask, center_mask, radius_mask, 255, cv::FILLED);

	cv::Mat elaborateHolesImage(realImage.size(), realImage.type());
	elaborateHolesImage.setTo(0); 
	realImage.copyTo(elaborateHolesImage, mask);
	return elaborateHolesImage;
}

std::vector<float> Processing::findRadiusHolesCountours(cv::Mat imageThreshold)
{

	std::vector<std::vector<cv::Point> > contours;
	std::vector<cv::Vec4i> hierarchy;
	cv::findContours(imageThreshold, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE, cv::Point(0, 0));

	std::vector<std::vector<cv::Point> > holesContours;
	
	for (size_t i = 0; i < contours.size(); i++)
	{
		if (contours[i].size() < 20) // PARAMETRO 20 DA TENERE FISSO
			holesContours.push_back(contours.at(i));
	}
	
	std::vector<std::vector<cv::Point> > contours_poly(holesContours.size());
	std::vector<cv::Point2f>center(holesContours.size());
	std::vector<float>radius(holesContours.size());
	std::vector<GeometricCircle> holes;

	cv::Mat drawingHoles = cv::Mat::zeros(imageThreshold.size(), CV_8UC3);
	
	for (size_t i = 0; i < holesContours.size(); i++)
	{
		approxPolyDP(cv::Mat(holesContours[i]), contours_poly[i], 3, false);
		cv::minEnclosingCircle((cv::Mat)contours_poly[i], center[i], radius[i]);
		
		circle(drawingHoles, center[i], radius[i], cv::Scalar(255, 255, 255), 5,8,0);
		if (!center.empty())
			holes.push_back(GeometricCircle(center[i], radius[i]));
	}

	float maxHoleRadius = Math::takeMax(radius);
	float minHoleRadius = Math::takeMin(radius);
	int indexMaxRadius = Math::index(maxHoleRadius, radius);
	int indexMinRadius = Math::index(minHoleRadius, radius);
	
	circle(drawingHoles, center[indexMinRadius], radius[indexMinRadius], cv::Scalar(0, 255, 0), 5, 8, 0);
	circle(drawingHoles, center[indexMaxRadius], radius[indexMaxRadius], cv::Scalar(255, 0, 0), 5, 8, 0);
	
	return radius;
}

GeometricCircle Processing::findFittingCircle(cv::Mat imageThreshold)
{
	std::vector<std::vector<cv::Point> > contours;
	std::vector<cv::Vec4i> hierarchy;
	cv::findContours(imageThreshold, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE, cv::Point(0, 0));

	std::vector<std::vector<cv::Point> > outerContours;
	for (size_t i = 0; i < contours.size(); i++)
	{
		if (contours[i].size() > 20) // PARAMETRO 20 DA TENERE FISSO
			outerContours.push_back(contours.at(i));
	}

	std::vector<std::vector<cv::Point> > contours_poly(outerContours.size());
	std::vector<cv::Point2f>center(outerContours.size());
	std::vector<float>radius(outerContours.size());
	std::vector<GeometricCircle> outerCircles;

	cv::Mat drawingOuterCircles = cv::Mat::zeros(imageThreshold.size(), CV_8UC3);
		
	for (size_t i = 0; i < outerContours.size(); i++)
	{
		approxPolyDP(cv::Mat(outerContours[i]), contours_poly[i], 3, false);
		cv::minEnclosingCircle((cv::Mat)contours_poly[i], center[i], radius[i]);
		circle(drawingOuterCircles, center[i], radius[i], cv::Scalar(0, 255, 255), 5,8,0);
		if (!center.empty())
			outerCircles.push_back(GeometricCircle(center[i], radius[i]));
	}
	double dim = DBL_MAX;
	int minIndex = 0;

	for (size_t i = 0; i < outerCircles.size(); i++)
	{
		if (dim > outerCircles[i].radius() && outerCircles[i].radius() > 200) // 200 PARAMETRO FISSO PER INDIVIDUARE I DUE CERCHI PIU GRANDI
			minIndex = i;

	}
	return outerCircles[minIndex];
}

std::vector<float> Processing::radiusConversion(std::vector<float> radiusVector, float conversion)
{
	std::vector<float> diamHoles;
	for (size_t i = 0; i < radiusVector.size(); i++)
		diamHoles.push_back(radiusVector.at(i) * conversion * 2);

	return diamHoles;
}

void Processing::plot_gaussian(std::vector<float> diam, float maxHoleDiam, float minHoleDiam)
{
	int dim = diam.size();
	std::vector<double> x(dim);
	std::vector<double> density(dim);

	float mu = bst::mean(diam); // MEDIA
	float varianza = bst::variance(diam);
	float sigma = sqrt(varianza); // DEVIAZIONE STANDARD 

	float tolleranza = 0.33 * 40 / 100;
	float min_range = 0.33 - tolleranza;
	float max_range = 0.33 + tolleranza;

	for (int i = 0; i <= dim; i++)
	{
		x[i] = (max_range - min_range) / 10 * i; // 10 PER ORA COSTANTE FISSA, 10 INTERVALLINI 

		density[i] = 1.0 / (sigma * sqrt(2.0 * 3.14)) * exp(-(pow((x[i] - mu) / sigma, 2) / 2.0));
	}

	plt::plot(x, density, "g");
	plt::show();
}




