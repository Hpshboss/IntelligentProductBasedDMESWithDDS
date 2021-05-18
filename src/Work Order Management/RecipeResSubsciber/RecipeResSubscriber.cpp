// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");

// Hpshboss modifys code from eprosima's github example;
// Licensed under the Apache License, Version 2.0 (the "License");

/**
 * @file Publisher.cpp
 *
 */
#include "recipeResPubSubTypes.h"

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

#include "recipeRes.h"

#include "RecipeResSubscriber.h"

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;
using namespace eprosima::fastrtps::types;

// using namespace cv;

RecipeResSubscriber::RecipeResSubscriber(std::shared_ptr<spdlog::logger> logger)
    : mp_participant(nullptr)
    , mp_subscriber(nullptr)
    , Logger(logger)
{
    m_listener.LLogger = Logger;
}


RecipeResSubscriber::~RecipeResSubscriber()
{
    Domain::removeParticipant(mp_participant);

    Domain::stopAll();
}


bool RecipeResSubscriber::init()
{
    ParticipantAttributes PParam;
    PParam.rtps.setName("Participant_sub_recipeRes");

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
    Rparam.topic.topicDataType = "recipeRes";
    Rparam.topic.topicName = "recipeRes";
    Rparam.qos.m_reliability.kind = BEST_EFFORT_RELIABILITY_QOS;
    mp_subscriber = Domain::createSubscriber(mp_participant, Rparam, (SubscriberListener*)&m_listener);

    if (mp_subscriber == nullptr)
    {
        return false;
    }

    return true;
}

void RecipeResSubscriber::SubListener::onSubscriptionMatched(
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

void RecipeResSubscriber::PartListener::onParticipantDiscovery(
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

void RecipeResSubscriber::SubListener::onNewDataMessage(
        Subscriber* sub)
{
    if (sub->takeNextData(&m_recipeRes, &m_info))
    {
        if (m_info.sampleKind == ALIVE)
        {
            std::string GUID;
            GUID = m_recipeRes.GUID();
            LLogger->debug("Order Number: " + GUID + "; \t");

            unsigned int orderNumber;
            orderNumber = m_recipeRes.orderNumber();
            LLogger->debug("Order Number: " + std::to_string(orderNumber) + "; \t");

            unsigned int orderPosition;
            orderPosition = m_recipeRes.orderPosition();
            LLogger->debug("Quantity: " + std::to_string(orderPosition) + "; \t");

            LLogger->debug("Note: " + m_recipeRes.note() + "; \t");
        }
    }
}

