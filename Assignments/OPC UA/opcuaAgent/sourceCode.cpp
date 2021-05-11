#include <iostream>
#include <stdexcept>
#include <thread>

#include "festoLab.h"

using namespace OpcUa;

int main(int argc, char ** argv){
    std::shared_ptr<spdlog::logger> logger = spdlog::stderr_color_mt("client");
    //logger->set_level(spdlog::level::debug); 
    try
    {
        festoLab::MagBack magBack(logger);

        short data = 3;
        std::cout << "first: " << magBack.readRfid() << std::endl;
        magBack.writeRfid(data);
        std::cout << "second: " << magBack.readRfid() << std::endl;
        magBack.writeRfid((short)10);
        std::cout << "third: " << magBack.readRfid() << std::endl;
    }

    catch (const std::exception & exc)
    {
        logger->error("Error in main: {}", exc.what());
    }

    catch (...)
    {
        logger->error("Unknown error in main.");
    }

    return -1;
}