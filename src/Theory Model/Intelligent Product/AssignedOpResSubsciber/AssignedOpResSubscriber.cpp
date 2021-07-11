// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");

// Hpshboss modifys code from eprosima's github example;
// Licensed under the Apache License, Version 2.0 (the "License");

/**
 * @file Publisher.cpp
 *
 */
#include "assignedOpResPubSubTypes.h"

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

#include "assignedOpRes.h"

#include "AssignedOpResSubscriber.h"

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;
using namespace eprosima::fastrtps::types;

// using namespace cv;

AssignedOpResSubscriber::AssignedOpResSubscriber(std::shared_ptr<spdlog::logger> logger)
    : mp_participant(nullptr)
    , mp_subscriber(nullptr)
    , Logger(logger)
{
    m_listener.LLogger = Logger;
}


AssignedOpResSubscriber::~AssignedOpResSubscriber()
{
    Domain::removeParticipant(mp_participant);

    Domain::stopAll();
}


bool AssignedOpResSubscriber::init()
{
    ParticipantAttributes PParam;
    PParam.rtps.setName("Participant_sub_assignedOpRes");

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
    Rparam.topic.topicDataType = "assignedOpRes";
    Rparam.topic.topicName = "assignedOpRes";
    Rparam.qos.m_reliability.kind = BEST_EFFORT_RELIABILITY_QOS;
    mp_subscriber = Domain::createSubscriber(mp_participant, Rparam, (SubscriberListener*)&m_listener);

    if (mp_subscriber == nullptr)
    {
        return false;
    }

    return true;
}

void AssignedOpResSubscriber::SubListener::onSubscriptionMatched(
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

void AssignedOpResSubscriber::PartListener::onParticipantDiscovery(
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

void AssignedOpResSubscriber::SubListener::onNewDataMessage(
        Subscriber* sub)
{
    if (sub->takeNextData(&m_assignedOpRes, &m_info))
    {
        if (m_info.sampleKind == ALIVE)
        {
            m_messageStack = true;

            /*
            unsigned int resourceId;
            resourceId = m_assignedOpRes.resourceId();

            unsigned int portId;
            portId = m_assignedOpRes.portId();

            std::string GUID;
            GUID = m_assignedOpRes.GUID();

            unsigned int carrierId;
            carrierId = m_assignedOpRes.carrierId();

            std::string operationInfo;
            operationInfo = m_assignedOpRes.operationInfo();

            std::string result;
            result = m_assignedOpRes.result();

            LLogger->debug("resourceId: " + std::to_string(resourceId) + "; \t" +
                           "portId: " + std::to_string(portId) + "; \t" +
                           "GUID: " + GUID + "; \t" +
                           "Carrier ID: " + std::to_string(carrierId) + "; \t" +
                           "operationInfo: " + operationInfo + "; \t" +
                           "result: " + result + "; \t" +
                           "Note: " + m_assignedOpRes.note() + "; \t");
            */
        }
    }
}

