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
	std::cout << typeid(*this).name() << "::" << __func__ << " END\n";
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

void Grabbing::loadPath(std::string imageFilePath)
{
	std::lock_guard<std::mutex> guard(m_imageFilePathMutex);
	do {
		if (m_imageFilePath != imageFilePath)
			break;

		return;

	} while (0);
	
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

void Grabbing::setImageEmpty(bool imageMode)
{
	std::lock_guard<std::mutex> guard(m_imageMutex);
	m_imageEmpty = imageMode;
}


bool Grabbing::stopFlag()
{
	std::lock_guard<std::mutex> guard(m_stop_mutex);
	return m_stop_flag;
}

void Grabbing::run()
{
	std::cout << typeid(*this).name() << "::" << __func__ << " START\n";
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
	std::cout << typeid(*this).name() << "::" << __func__ << " END\n";
}


void Grabbing::loadImagesFromDisk()
{
	std::unique_lock<std::mutex> fileGuard(m_imageFilePathMutex);

	do
	{
		if (m_imageFilePathNew)
			break;

		std::cv_status res = m_imageFilePathReady.wait_for(fileGuard, std::chrono::milliseconds(100));
		if (res == std::cv_status::no_timeout)
			break;

		return;
	} while (0);
	

	std::lock_guard<std::mutex> imageGuard(m_imageMutex);
	std::cout << " --> " << typeid(*this).name() << "::" << __func__ << " file path modified" << "\n";

	m_image = cv::imread(m_imageFilePath);
	std::cout << "    - " << "Image: " << " - " << m_image.size() << "\n";

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

bool Grabbing::isImageEmpty()
{
	std::lock_guard<std::mutex> guard(m_imageMutex);
	return m_imageEmpty;
}


