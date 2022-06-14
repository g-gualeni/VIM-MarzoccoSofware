#include "Grabbing.h"

#include <iostream>
#include <fstream>



Grabbing::Grabbing()
{
	start();
}

Grabbing::~Grabbing()
{
	stopAndWait();
}

void Grabbing::start()
{
	std::lock_guard<std::mutex> guard(m_stop_mutex);
	m_stop_flag = false;
	m_thread = new std::thread(&Grabbing::run, this);
}

void Grabbing::stopAndWait()
{
	m_stop_mutex.lock();
	m_stop_flag = true;
	m_stop_mutex.unlock();
	m_thread->join();
	std::cout << typeid(*this).name() << "::" << __func__ << " end\n\n";
}


bool Grabbing::isImageFileMode()
{
	std::lock_guard<std::mutex> guard(m_imageFilePathMutex);
	return m_imageFileMode;
}

void Grabbing::setImageFileMode(bool fileMode)
{
	std::lock_guard<std::mutex> guardImageFile(m_imageFilePathMutex);
	m_imageFileMode = fileMode;
	std::lock_guard<std::mutex> guardImage(m_imageMutex);
	m_imageEmpty = true;

}

bool Grabbing::getImageFileMode()
{
	return m_imageFileMode;
}

void Grabbing::loadPath(std::string imageFilePath)
{
	std::lock_guard<std::mutex> guard(m_imageFilePathMutex);
	if (m_imageFilePath != imageFilePath)
		return;
	
	m_imageFilePath = imageFilePath;
	m_imageFilePathNew = true;
	m_imageFileMode = true;
	m_imageFilePathReady.notify_all();
}

cv::Mat Grabbing::imageWait(int timeout_ms)
{
	int count = (int)std::round(timeout_ms / 1000.0);
	while (stopFlag() == false)
	{
		std::unique_lock<std::mutex> lock(m_imageMutex);
		if (m_imageEmpty == false)
			return m_image;
		
		std::cv_status res = m_imageReady.wait_for(lock, std::chrono::milliseconds(1000));
		if (res == std::cv_status::no_timeout)
		{
			if (m_imageEmpty == false)
				return m_image;
		}

		count--;
		if (count <= 0)
			break;
	}
	return cv::Mat();
}


bool Grabbing::stopFlag()
{
	std::lock_guard<std::mutex> guard(m_stop_mutex);
	return m_stop_flag;
}

void Grabbing::run()
{
	std::cout << typeid(*this).name() << "::" << __func__ << " start\n\n";
	while (!stopFlag())
	{
		if (isImageFileMode())
		{
			loadImagesFromDisk();
		}
		else
		{
			loadImagesFromCamera();
		}
	}
}


void Grabbing::loadImagesFromDisk()
{
	std::unique_lock<std::mutex> fileGuard(m_imageFilePathMutex);

	if (m_imageFilePathNew)
		return;
	
	std::cv_status res = m_imageFilePathReady.wait_for(fileGuard, std::chrono::milliseconds(100));
	if (res == std::cv_status::no_timeout)
		return;

	std::lock_guard<std::mutex> imageGuard(m_imageMutex);
	std::cout << " --> " << typeid(*this).name() << "::" << __func__ << " file path modified" << "\n";

	std::ifstream image_buffer(m_imageFilePath, std::ios::binary);
	image_buffer.seekg(0); // DA MODIFICARE
	std::streampos start = image_buffer.tellg();
	image_buffer.seekg(0, std::ios::end);
	std::streampos end = image_buffer.tellg();
	auto image_buffer_size = end - start;
	image_buffer.seekg(0, std::ios::beg);
	char* buffer = new char[image_buffer_size];
	image_buffer.read(buffer, image_buffer_size);
	cv::Mat img = cv::Mat(3008, 4112, CV_16U, buffer);
	m_image = conversione(img);

	cv::namedWindow("Output Image", cv::WINDOW_AUTOSIZE);
	cv::imshow("Output Image", img);
	cv::waitKey();

	image_buffer.close();
	//delete buffer;
	
	m_imageFilePathNew = false;
	m_imageEmpty = false;

	if (m_image.empty())
		m_imageEmpty = true;
	
	if (m_imageEmpty == false)
		m_imageReady.notify_all();
}

void Grabbing::loadImagesFromCamera()
{
}

cv::Mat Grabbing::conversione(cv::Mat img)
{
	double min = 0;
	double max = 65536;
	cv::minMaxLoc(img, &min, &max);
	cv::Mat img8bit;
	img.convertTo(img8bit, CV_8U, 255.0 / (max - min), -min * 255.0 / (max - min));
	cv::Mat BGRimage;
	cv::cvtColor(img8bit, BGRimage, cv::COLOR_GRAY2BGR);
	return BGRimage;
}
