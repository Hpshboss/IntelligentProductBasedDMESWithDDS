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

#include "AssignedOpSubscriber.h"
#include "CarrierPosPublisher.h"
#include "AssignedOpResPublisher.h"


using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;
using namespace eprosima::fastrtps::types;

class ASRSMachine
{
public:
    ASRSMachine(std::shared_ptr<spdlog::logger> logger) 
        : Logger(logger)
        , assignedOpSubscriber(logger)
        , carrierPosPublisher(logger)
        , assignedOpResPublisher(logger)
    {
        carrierPosPublisher.init();
        assignedOpResPublisher.init();
    };

    ~ASRSMachine()
    {

    };

    bool monitorAssignedOperation()
    {
        assignedOpSubscriber.init();
    };

    bool broadcastCarrierPosition(unsigned int resourceId, unsigned int portId, unsigned int carrierId, std::string note)
    {
        carrierPosPublisher.publish(resourceId, portId, carrierId, note);
    };

    bool responseAssignedOperation(unsigned int resourceId, unsigned int portId, std::string GUID, unsigned int carrierId, std::string operationInfo, std::string result, std::string note)
    {
        assignedOpResPublisher.publish(resourceId, portId, GUID, carrierId, operationInfo, result, note);
    };


private:
    Common::Logger::SharedPtr Logger;
    AssignedOpSubscriber assignedOpSubscriber;
    CarrierPosPublisher carrierPosPublisher;
    AssignedOpResPublisher assignedOpResPublisher;
};