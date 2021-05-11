#include <iostream>
#include <stdexcept>
#include <thread>

#include "festoLab.h"

using namespace OpcUa;

int main(int argc, char ** argv){
    std::shared_ptr<spdlog::logger> logger = spdlog::stderr_color_mt("client");
    logger->set_level(spdlog::level::debug); 
    try
    {
        festoLab::MagBack magBack(logger);
        bool gIsStop = false;
        magBack.monitorCarrierArrivalThenStop(&gIsStop);

        while(true)
        {
            logger->debug("keep other thing. {}", std::to_string(gIsStop));
            std::this_thread::sleep_for(std::chrono::milliseconds(2000));
            if (gIsStop) 
            {
                logger->debug("RFID value is {}", magBack.readRfid());
                std::this_thread::sleep_for(std::chrono::milliseconds(1500));
                magBack.automatic();
                std::this_thread::sleep_for(std::chrono::milliseconds(1500));
                gIsStop = false;
                magBack.monitorCarrierArrivalThenStop(&gIsStop);
            }
        }
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