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
bool thread_created = false;
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
			thread_created = true;
			std::cout << "[MAIN] TIMER THREAD TERMINATO PRIMA DEL TIMEOUT!!! " << "\n";
			return;
		}
		Sleep(200);
		contatore++;
	}
	mainTimerMutex.lock();
	main_timer = true;
	mainTimerMutex.unlock();
	thread_created = true;
	std::cout << "[MAIN] THREAD TERMINATO!!! TIMEOUT SCADUTO " << "\n";
	if(!checker->getZeroZone_check())
		std::cout << "[MAIN] Ingombro! Si prega l'operatore di liberare la zona di ispezione!\n";
	else if(!checker->getFirstZone_check())
		std::cout << "[MAIN] Tempo di attesa scaduto! Si prega l'operatore di posizionare meglio il pezzo!\n";
};
void alert_message(Checking* checker)
{
	int contatore = 0;
	if (checker->getBoolTimer())
	{
		if (main_timer)
		{
			timer = new std::thread(worker, checker);
		}
				
		main_timer = false;
		
	}
	
	
}
int main()
{
    std::cout << "USING CV VERSION: " << cv::getVersionString() << "\n";
    std::cout << "USING BOOST VERSION: " << BOOST_LIB_VERSION << "\n";
    std::cout << "USING NLOHMANN JSON VERSION: " << NLOHMANN_JSON_VERSION_MAJOR << "." << NLOHMANN_JSON_VERSION_MINOR << "." << NLOHMANN_JSON_VERSION_PATCH << "\n";
    std::cout << "USING WEBSOCKET VERSION: " << websocketpp::user_agent << "\n\n";
		
	bool checkFileMode = false;
	bool saveCheck = false;

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
				std::cout << "[MAIN] Inserire un pezzo da analizzare\n";
				Sleep(5000);				//Attesa che l'utente inserisca un pezzo sotto l'illuminatore
			}
			alert_message(checker);
														

		} while (!checker->getZeroZone_check() || !checker->getFirstZone_check());

		std::cout << "\n[MAIN] Pezzo in posizione!\n\n";
		grabber->setStopGrabbing(true);
		processer->setImage(imageGrabberWait);

		cv::Mat outImage = processer->imageOutputWait();
		bool firstPrint = true;
		grabber->setStopGrabbing(false);
		do
		{
			checker->ZonesChecker();
			if (firstPrint)
				std::cout << "[MAIN] ATTESA DELL'EVENTO DI INGROMBRO " << "\n";
			firstPrint = false;

		} while (checker->getZeroZone_check());
		
		std::cout << "[MAIN] INGROMBRO AVVENUTO. PROSSIMA ACQUISIZIONE " << "\n";
		checker->setZeroZone_check(false);
		checker->setFirstZone_check(false);
		
	}
		
	std::cout << "\n\n[MAIN]: Applicazione finita\n\n";
	delete processer;
	delete grabber;
	delete checker;
	aspettaESC.join();

    return 0;
}


