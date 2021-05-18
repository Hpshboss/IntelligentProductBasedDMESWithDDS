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

#include "WorkOrderManagement.h"


int main(int argc, char ** argv){
    std::shared_ptr<spdlog::logger> logger = spdlog::stderr_color_mt("client");
    logger->set_level(spdlog::level::debug); 
    try
    {
        WorkOrderManagement workOrderManagement(logger);

        workOrderManagement.monitorPlacedOrder();
        workOrderManagement.monitorRecipeRes();
        workOrderManagement.monitorProductReport();

        std::this_thread::sleep_for(std::chrono::milliseconds(1500));
        workOrderManagement.assignRecipeInfo(3001, 1, "210:12:5;310:5:5;", "Nope");
        std::this_thread::sleep_for(std::chrono::milliseconds(1500));
        workOrderManagement.assignRecipeInfo(3001, 2, "210:12:5;310:5:5;", "Nope");
        std::this_thread::sleep_for(std::chrono::milliseconds(1500));
        workOrderManagement.assignRecipeInfo(3001, 3, "210:12:5;310:5:5;", "Nope");

        while(true)
        {
            logger->debug("keep running...");
            std::this_thread::sleep_for(std::chrono::milliseconds(1500));
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