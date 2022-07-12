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

std::mutex mainTimerMutex;
bool main_timer = true;
bool threadTimeout = false;
std::thread* timer = nullptr;

bool checkEscRequest()
{
	std::lock_guard<std::mutex> guard(EscFlagMutex);
	return EscFlag;
}

std::string referenceImageFolder()
{
	std::filesystem::path referenceImageFolder("C:\\GitHub\\VIM-MarzoccoSofware\\server\\Res\\Con_filtro.tiff");
	return referenceImageFolder.string();

}
auto worker = [](Checking* checker) {

	int contatore = 0;
	
	while (contatore < 25)
	{
		if (checker->getFirstZone_check())
		{
			mainTimerMutex.lock();
			main_timer = true;
			mainTimerMutex.unlock();
			threadTimeout = true;
			return;
		}
		Sleep(200);
		contatore++;
	}
	mainTimerMutex.lock();
	main_timer = true;
	mainTimerMutex.unlock();
	threadTimeout = true;
	if(!checker->getZeroZone_check())
		std::cout << "Ingombro! Si prega l'operatore di liberare la zona di ispezione!\n";
	else if(!checker->getFirstZone_check())
		std::cout << "Tempo di attesa scaduto! Si prega l'operatore di posizionare meglio il pezzo!\n";
};
void alert_message(Checking* checker)
{
	int contatore = 0;
	if (checker->getBoolTimer())
	{
		if (main_timer)
			timer = new std::thread(worker, checker);
				
		main_timer = false;
		
	}
	
	
}
int main()
{
    std::cout << "USING CV VERSION: " << cv::getVersionString() << "\n";
    std::cout << "USING BOOST VERSION: " << BOOST_LIB_VERSION << "\n";
    std::cout << "USING NLOHMANN JSON VERSION: " << NLOHMANN_JSON_VERSION_MAJOR << "." << NLOHMANN_JSON_VERSION_MINOR << "." << NLOHMANN_JSON_VERSION_PATCH << "\n";
    std::cout << "USING WEBSOCKET VERSION: " << websocketpp::user_agent << "\n";
		
	bool checkFileMode = true;
	bool safe_check = false;

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
	if (checkFileMode)
		grabber->loadPath(image_path);

	grabber->setImageFileMode(checkFileMode);

	while (checkEscRequest() == false)
	{
		
		if (checkFileMode)
			EscFlag = true;
			
		cv::Mat imageGrabberWait; 

		do
		{
			imageGrabberWait = grabber->imageWait(3000);
			checker->setImage(imageGrabberWait);
			checker->ZonesChecker();
			if (!checker->getBoolTimer() && !checker->getZeroZone_check())
			{
				std::cout << "Inserire un pezzo da analizzare\n";
				Sleep(5000);				//Attesa che l'utente inserisca un pezzo sotto l'illuminatore
			}
			alert_message(checker);
			if (threadTimeout)
			{
				timer->join();
				safe_check = true;
			}
									

		} while (!checker->getZeroZone_check() || !checker->getFirstZone_check());

		std::cout << "Pezzo in posizione!\n";
		processer->setImage(imageGrabberWait);

		cv::Mat outImage = processer->imageOutputWait();
		checker->setZeroZone_check(false);
		checker->setFirstZone_check(false);

			

	}

	
	std::cout << "\n\n[MAIN]: Applicazione finita\n\n";
	if (!safe_check && timer != nullptr)
		timer->join();
	delete processer;
	delete grabber;
	delete checker;
	aspettaESC.join();

    return 0;
}


