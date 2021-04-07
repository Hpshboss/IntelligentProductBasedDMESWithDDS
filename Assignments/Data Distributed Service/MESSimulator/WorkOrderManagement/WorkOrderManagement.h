// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");

// Hpshboss modifys code from eprosima's github example;
// Licensed under the Apache License, Version 2.0 (the "License");

/**
 * @file Subscriber.cpp
 *
 */
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/attributes/SubscriberAttributes.h>
#include <fastrtps/subscriber/Subscriber.h>
#include <fastrtps/Domain.h>

#include <thread>
#include <time.h>
#include <vector>
#include <string> 

// #include <spglog/spdlog.h>
// #include <fastrtps/log/Log.h>
#include <opc/common/logger.h>

#include "OrderInfoSubscriber.h"

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;
using namespace eprosima::fastrtps::types;

class WorkOrderManagement
{
public:
    WorkOrderManagement(std::shared_ptr<spdlog::logger> logger) : Logger(logger), orderInfoSubscriber(logger)
    {
        orderInfoSubscriber.init();
    };

    ~WorkOrderManagement()
    {

    };

    bool monitorPlacedOrder()
    {
    };

private:
    Common::Logger::SharedPtr Logger;
    OrderInfoSubscriber orderInfoSubscriber;
    
};