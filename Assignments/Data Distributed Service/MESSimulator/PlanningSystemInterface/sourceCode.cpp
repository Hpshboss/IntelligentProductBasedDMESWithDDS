// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");

// Hpshboss modifys code from eprosima's github example;
// Licensed under the Apache License, Version 2.0 (the "License");

/**
 * @file source.cpp
 *
 */

#include "Publisher.h"
#include "Subscriber.h"

#include <fastrtps/Domain.h>
#include <fastrtps/log/Log.h>

#include <fstream>

using namespace eprosima;
using namespace fastrtps;
using namespace rtps;

#include <iostream>
#include <stdexcept>
#include <thread>


using namespace OpcUa;

int main(int argc, char ** argv){
    std::shared_ptr<spdlog::logger> logger = spdlog::stderr_color_mt("client");
    //logger->set_level(spdlog::level::debug); 
    try
    {
        PlanningSystemInterfacePublisher mypub;
        if(mypub.init())
        {
            mypub.run(count, sleep);
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