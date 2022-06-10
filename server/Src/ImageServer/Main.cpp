

#include <opencv2/opencv.hpp>
#include <boost/version.hpp>
#include <nlohmann/json.hpp>
#include <websocket/version.hpp>
#include <iostream>

int main()
{
    std::cout << "USING CV VERSION: " << cv::getVersionString() << "\n";
    std::cout << "USING BOOST VERSION: " << BOOST_LIB_VERSION << "\n";
    std::cout << "USING NLOHMANN JSON VERSION: " << NLOHMANN_JSON_VERSION_MAJOR << "." << NLOHMANN_JSON_VERSION_MINOR << "." << NLOHMANN_JSON_VERSION_PATCH << "\n";
    std::cout << "USING WEBSOCKET VERSION: " << websocketpp::user_agent << "\n";

    std::cout << "Hello World!\n";
}


