#include "GRABBING/Grabbing.h"
#include "PROCESSING/Processing.h"
#include "GUI/GUICommunication.h"
#include "CHECKING/Checking.h"

#include <opencv2/opencv.hpp>
#include <boost/version.hpp>
#include <nlohmann/json.hpp>
#include <websocket/version.hpp>

#include <iostream>
#include <Windows.h>
#include <WinUser.h>
#include <filesystem>


std::mutex EscFlagMutex;
bool EscFlag = false;
bool checkEscRequest()
{
	std::lock_guard<std::mutex> guard(EscFlagMutex);
	return EscFlag;
}

std::string referenceImageFolder()
{
	std::filesystem::path referenceImageFolder("C:\\GitHub\\VIM-MarzoccoSofware\\server\\Res\\no_filtro.tiff");
	return referenceImageFolder.string();

}
int main()
{
    std::cout << "USING CV VERSION: " << cv::getVersionString() << "\n";
    std::cout << "USING BOOST VERSION: " << BOOST_LIB_VERSION << "\n";
    std::cout << "USING NLOHMANN JSON VERSION: " << NLOHMANN_JSON_VERSION_MAJOR << "." << NLOHMANN_JSON_VERSION_MINOR << "." << NLOHMANN_JSON_VERSION_PATCH << "\n";
    std::cout << "USING WEBSOCKET VERSION: " << websocketpp::user_agent << "\n";
		
	bool checkFileMode = true;

	std::thread aspettaESC([&]() {
		// WAIT for the ESC key and then set the EscFlag to stop execution
		while (GetAsyncKeyState(VK_ESCAPE) == 0) {

			if (checkEscRequest())
				return;

			Sleep(200);
		}
		std::cout << "--> Premuto ESC\n";
		EscFlagMutex.lock();
		EscFlag = true;
		EscFlagMutex.unlock();
		std::cout << "--> ESC Thread finito\n";
		});
    Grabbing* grabber = new Grabbing();
	Checking* checker = new Checking();
    Processing* processer = new Processing();

	std::string image_path = referenceImageFolder();
	grabber->loadPath(image_path);
	grabber->setImageFileMode(checkFileMode);

	
	while (checkEscRequest() == false)
	{

		if (checkFileMode)
			EscFlag = true;

		cv::Mat imageGrabberWait = grabber->imageWait(3000);
		checker->setImage(imageGrabberWait);

		cv::Mat imageCheckerWait = checker->imageWait(3000);

	}

	
	std::cout << "\n\n[MAIN]: Applicazione finita\n\n";
	delete processer;
	delete checker;
	delete grabber;
	aspettaESC.join();

    return 0;
}


