// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");

// Hpshboss modifys code from eprosima's github example;
// Licensed under the Apache License, Version 2.0 (the "License");

/**
 * @file Publisher.cpp
 *
 */
#include "assignedOpPubSubTypes.h"

#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/attributes/PublisherAttributes.h>
#include <fastrtps/subscriber/Subscriber.h>
#include <fastrtps/Domain.h>

#include <fastrtps/TopicDataType.h>

#include <thread>
#include <time.h>
#include <vector>
#include <string> 
#include <map>
#include <boost/any.hpp>

// #include <spglog/spdlog.h>
// #include <fastrtps/log/Log.h>
#include <opc/common/logger.h>

#include "assignedOp.h"

#include "AssignedOpSubscriber.h"

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;
using namespace eprosima::fastrtps::types;

// using namespace cv;

AssignedOpSubscriber::AssignedOpSubscriber(std::shared_ptr<spdlog::logger> logger)
    : mp_participant(nullptr)
    , mp_subscriber(nullptr)
    , Logger(logger)
{
    m_listener.LLogger = Logger;
}


AssignedOpSubscriber::~AssignedOpSubscriber()
{
    Domain::removeParticipant(mp_participant);

    Domain::stopAll();
}


bool AssignedOpSubscriber::init()
{
    ParticipantAttributes PParam;
    PParam.rtps.setName("Participant_sub_assignedOp");

    mp_participant = Domain::createParticipant(PParam, (ParticipantListener*)&m_part_list);

    if (mp_participant == nullptr)
    {
        return false;
    }

    //REGISTER THE TYPE
    Domain::registerType(mp_participant, &m_type);

    //CREATE THE SUBSCRIBER
    SubscriberAttributes Rparam;
    Rparam.topic.topicKind = NO_KEY;
    Rparam.topic.topicDataType = "assignedOp";
    Rparam.topic.topicName = "assignedOp";
    Rparam.qos.m_reliability.kind = BEST_EFFORT_RELIABILITY_QOS;
    mp_subscriber = Domain::createSubscriber(mp_participant, Rparam, (SubscriberListener*)&m_listener);

    if (mp_subscriber == nullptr)
    {
        return false;
    }

    return true;
}

void AssignedOpSubscriber::SubListener::onSubscriptionMatched(
        Subscriber* /*sub*/,
        MatchingInfo& info)
{
    if (info.status == MATCHED_MATCHING)
    {
        n_matched++;
        std::cout << "Subscriber matched" << std::endl;
    }
    else
    {
        n_matched--;
        std::cout << "Subscriber unmatched" << std::endl;
    }
}

void AssignedOpSubscriber::PartListener::onParticipantDiscovery(
        Participant*,
        ParticipantDiscoveryInfo&& info)
{
    if (info.status == ParticipantDiscoveryInfo::DISCOVERED_PARTICIPANT)
    {
        std::cout << "Participant " << info.info.m_participantName << " discovered" << std::endl;
    }
    else if (info.status == ParticipantDiscoveryInfo::REMOVED_PARTICIPANT)
    {
        std::cout << "Participant " << info.info.m_participantName << " removed" << std::endl;
    }
    else if (info.status == ParticipantDiscoveryInfo::DROPPED_PARTICIPANT)
    {
        std::cout << "Participant " << info.info.m_participantName << " dropped" << std::endl;
    }
}

void AssignedOpSubscriber::SubListener::onNewDataMessage(
        Subscriber* sub)
{
    if (sub->takeNextData(&m_assignedOp, &m_info))
    {
        if (m_info.sampleKind == ALIVE)
        {
            unsigned int resourceId;
            resourceId = m_assignedOp.resourceId();
            LLogger->debug("resourceId: " + std::to_string(resourceId) + "; \t");

            unsigned int portId;
            portId = m_assignedOp.portId();
            LLogger->debug("portId: " + std::to_string(portId) + "; \t");

            std::string GUID;
            GUID = m_assignedOp.GUID();
            LLogger->debug("GUID: " + GUID + "; \t");

            unsigned int carrierId;
            carrierId = m_assignedOp.carrierId();
            LLogger->debug("Carrier ID: " + std::to_string(carrierId) + "; \t");

            std::string operationInfo;
            operationInfo = m_assignedOp.operationInfo();
            LLogger->debug("operationInfo: " + operationInfo + "; \t");

            LLogger->debug("Note: " + m_assignedOp.note() + "; \t");
        }
    }
}

