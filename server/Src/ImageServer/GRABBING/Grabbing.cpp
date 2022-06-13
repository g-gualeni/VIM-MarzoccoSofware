#include "Grabbing.h"
#include <iostream>


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
	std::cout << "FileMode: " << fileMode;
	std::lock_guard<std::mutex> guardImage(m_imageMutex);
	m_imageEmpty = true;

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

	}
}