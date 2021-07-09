// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");

// Hpshboss modifys code from eprosima's github example;
// Licensed under the Apache License, Version 2.0 (the "License");

/**
 * @file Publisher.cpp
 *
 */
#include "orderInfoPubSubTypes.h"

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

#include "orderInfo.h"

#include "OrderInfoSubscriber.h"

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;
using namespace eprosima::fastrtps::types;

// using namespace cv;

OrderInfoSubscriber::OrderInfoSubscriber(std::shared_ptr<spdlog::logger> logger)
    : mp_participant(nullptr)
    , mp_subscriber(nullptr)
    , Logger(logger)
{
    m_listener.LLogger = Logger;
}


OrderInfoSubscriber::~OrderInfoSubscriber()
{
    Domain::removeParticipant(mp_participant);

    Domain::stopAll();
}


bool OrderInfoSubscriber::init()
{
    ParticipantAttributes PParam;
    PParam.rtps.setName("Participant_sub_orderInfo");

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
    Rparam.topic.topicDataType = "orderInfo";
    Rparam.topic.topicName = "orderInfo";
    Rparam.qos.m_reliability.kind = BEST_EFFORT_RELIABILITY_QOS;
    mp_subscriber = Domain::createSubscriber(mp_participant, Rparam, (SubscriberListener*)&m_listener);

    if (mp_subscriber == nullptr)
    {
        return false;
    }

    return true;
}

void OrderInfoSubscriber::SubListener::onSubscriptionMatched(
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

void OrderInfoSubscriber::PartListener::onParticipantDiscovery(
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

void OrderInfoSubscriber::SubListener::onNewDataMessage(
        Subscriber* sub)
{
    if (sub->takeNextData(&m_orderInfo, &m_info))
    {
        if (m_info.sampleKind == ALIVE)
        {
            m_messageStack = true;
            LLogger->debug("Receive an order");
            unsigned int orderNumber;
            orderNumber = m_orderInfo.orderNumber();
            // public_orderInfo.orderNumber(m_orderInfo.orderNumber());

            unsigned int partNumber;
            partNumber = m_orderInfo.partNumber();
            // public_orderInfo.partNumber(m_orderInfo.partNumber());

            unsigned short quantity;
            quantity = m_orderInfo.quantity();
            // public_orderInfo.quantity(m_orderInfo.quantity());

            // public_orderInfo.note(m_orderInfo.note());

            // LLogger->debug("Test Number in onNewDataMessage: " + std::to_string(testNumber));

            LLogger->debug("Order Number: " + std::to_string(orderNumber) + "; \t" + 
                            "Part Number: " + std::to_string(partNumber) + "; \t" +
                            "Quantity: " + std::to_string(quantity) + "; \t" +
                            "Note: " + m_orderInfo.note() + "; \t");
        }
    }
}

