// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");

// Hpshboss modifys code from eprosima's github example;
// Licensed under the Apache License, Version 2.0 (the "License");

/**
 * @file source.cpp
 *
 */

#include <fastrtps/Domain.h>
#include <fastrtps/log/Log.h>

#include <fstream>

using namespace eprosima;
using namespace fastrtps;
using namespace rtps;

#include <iostream>
#include <stdexcept>
#include <thread>
#include <string> 

// #include <spglog/spdlog.h>
#include <opc/common/logger.h>

#include "IntelligentProduct.h"


int main(int argc, char ** argv){
    std::shared_ptr<spdlog::logger> logger = spdlog::stderr_color_mt("client");
    logger->set_level(spdlog::level::debug); 
    try
    {
        IntelligentProduct intelligentProduct(logger);
        logger->debug("main");

        intelligentProduct.monitorCarrierPos();
        intelligentProduct.monitorRecipeInfo();
        intelligentProduct.monitorAssignedOpRes();

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        intelligentProduct.responseRecipe("21EC2020-3AEA-1069-A2DD-08002B303099", 3001, 1, "Nope");
        std::this_thread::sleep_for(std::chrono::milliseconds(1500));
        intelligentProduct.responseRecipe("21EC2020-3AEA-1069-A2DD-08002B30309A", 3001, 2, "Nope");
        std::this_thread::sleep_for(std::chrono::milliseconds(1500));
        intelligentProduct.responseRecipe("21EC2020-3AEA-1069-A2DD-08002B30309B", 3001, 3, "Nope");
        std::this_thread::sleep_for(std::chrono::milliseconds(1500));
        intelligentProduct.responseRecipe("21EC2020-3AEA-1069-A2DD-08002B30309C", 3001, 4, "Nope");
        std::this_thread::sleep_for(std::chrono::milliseconds(1500));
        intelligentProduct.responseRecipe("21EC2020-3AEA-1069-A2DD-08002B30309D", 3001, 5, "Nope");

        std::this_thread::sleep_for(std::chrono::milliseconds(1500));
        intelligentProduct.assignOperation(30, 1, "21EC2020-3AEA-1069-A2DD-08002B303099", 1, "210:30:32;", "Nope");
        std::this_thread::sleep_for(std::chrono::milliseconds(1500));
        intelligentProduct.assignOperation(30, 1, "21EC2020-3AEA-1069-B2DD-08002B303099", 2, "210:30:32;", "Nope");
        std::this_thread::sleep_for(std::chrono::milliseconds(1500));
        intelligentProduct.assignOperation(30, 1, "21EC2020-3AEA-1069-C2DD-08002B303099", 3, "210:30:32;", "Nope");
        std::this_thread::sleep_for(std::chrono::milliseconds(1500));
        intelligentProduct.assignOperation(30, 1, "21EC2020-3AEA-1069-D2DD-08002B303099", 4, "210:30:32;", "Nope");
        std::this_thread::sleep_for(std::chrono::milliseconds(1500));
        intelligentProduct.assignOperation(30, 1, "21EC2020-3AEA-1069-E2DD-08002B303099", 5, "210:30:32;", "Nope");

        std::this_thread::sleep_for(std::chrono::milliseconds(1500));
        intelligentProduct.reportProductResult("21EC2020-3AEA-1069-A2DD-08002B303099", 3001, 1, "DONE", "Nope");
        std::this_thread::sleep_for(std::chrono::milliseconds(1500));
        intelligentProduct.reportProductResult("21EC2020-3AEA-1069-A2DD-08002B303099", 3001, 2, "DONE", "Nope");
        std::this_thread::sleep_for(std::chrono::milliseconds(1500));
        intelligentProduct.reportProductResult("21EC2020-3AEA-1069-A2DD-08002B303099", 3001, 3, "DONE", "Nope");

        while(true)
        {
            if (*intelligentProduct.assignedOpResSubscriber.public_messageStack == true)
            {
                *intelligentProduct.assignedOpResSubscriber.public_messageStack = false;
                logger->debug("main GUID: " + intelligentProduct.assignedOpResSubscriber.public_assignedOpRes->GUID());
            }
            if (*intelligentProduct.carrierPosSubscriber.public_messageStack == true)
            {
                *intelligentProduct.carrierPosSubscriber.public_messageStack = false;
                logger->debug("main Carrier ID: " + std::to_string(intelligentProduct.carrierPosSubscriber.public_carrierPos->carrierId()));
            }
            if (*intelligentProduct.recipeInfoSubscriber.public_messageStack == true)
            {
                *intelligentProduct.recipeInfoSubscriber.public_messageStack = false;
                logger->debug("main Work Plan: " + intelligentProduct.recipeInfoSubscriber.public_recipeInfo->workPlan());
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
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