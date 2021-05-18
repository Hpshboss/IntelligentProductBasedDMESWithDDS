// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");

// Hpshboss modifys code from eprosima's github example;
// Licensed under the Apache License, Version 2.0 (the "License");

/**
 * @file Publisher.cpp
 *
 */
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/attributes/PublisherAttributes.h>
#include <fastrtps/publisher/Publisher.h>
#include <fastrtps/Domain.h>

#include <thread>
#include <time.h>
#include <vector>
#include <string> 

// #include <spglog/spdlog.h>
// #include <fastrtps/log/Log.h>
#include <opc/common/logger.h>

#include "OrderInfoPublisher.h"

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;
using namespace eprosima::fastrtps::types;

class PlanningSystemInterface
{
public:
    PlanningSystemInterface(std::shared_ptr<spdlog::logger> logger) : Logger(logger), orderInfoPublisher(logger)
    {
        orderInfoPublisher.init();
    };

    ~PlanningSystemInterface()
    {

    };

    bool placeAnOrder(unsigned int orderNumber, unsigned int partNumber, unsigned short quantity, std::string note)
    {
        Logger->debug("placeAnOrder");
        orderInfoPublisher.publish(orderNumber, partNumber, quantity, note);
    };

private:
    Common::Logger::SharedPtr Logger;
    OrderInfoPublisher orderInfoPublisher;
    
};