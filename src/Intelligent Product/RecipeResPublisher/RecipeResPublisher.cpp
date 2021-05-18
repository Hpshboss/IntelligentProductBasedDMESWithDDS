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
#include <fastrtps/publisher/Publisher.h>
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

#include "RecipeResPublisher.h"

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;
using namespace eprosima::fastrtps::types;

// using namespace cv;

RecipeResPublisher::RecipeResPublisher(std::shared_ptr<spdlog::logger> logger)
    : mp_participant(nullptr)
    , mp_publisher(nullptr)
    , Logger(logger)
{
}


RecipeResPublisher::~RecipeResPublisher()
{
    Domain::removeParticipant(mp_participant);

    Domain::stopAll();
}


bool RecipeResPublisher::init()
{
    ParticipantAttributes PParam;
    PParam.rtps.setName("Participant_pub_recipeRes");

    mp_participant = Domain::createParticipant(PParam, (ParticipantListener*)&m_part_list);

    if (mp_participant == nullptr)
    {
        return false;
    }

    //REGISTER THE TYPE
    Domain::registerType(mp_participant, &m_type);

    //CREATE THE PUBLISHER
    PublisherAttributes Wparam;
    Wparam.topic.topicKind = NO_KEY;
    Wparam.topic.topicDataType = "recipeRes";
    Wparam.topic.topicName = "recipeRes";
    Wparam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;

    mp_publisher = Domain::createPublisher(mp_participant, Wparam, (PublisherListener*)&m_listener);

    if (mp_publisher == nullptr)
    {
        return false;
    }

    return true;
}

void RecipeResPublisher::PubListener::onPublicationMatched(
        Publisher* /*pub*/,
        MatchingInfo& info)
{
    if (info.status == MATCHED_MATCHING)
    {
        n_matched++;
        std::cout << "Publisher matched" << std::endl;
    }
    else
    {
        n_matched--;
        std::cout << "Publisher unmatched" << std::endl;
    }
}

void RecipeResPublisher::PartListener::onParticipantDiscovery(
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

bool RecipeResPublisher::publish(std::string GUID, unsigned int orderNumber, unsigned int orderPosition, std::string note)
{
    Logger->debug("RecipeResPublisher::publish");
    try
    {
        if (m_listener.n_matched > 0) 
        {
            m_recipeRes.GUID(GUID);
            m_recipeRes.orderNumber(orderNumber);
            m_recipeRes.orderPosition(orderPosition);
            m_recipeRes.note(note);
            mp_publisher->write(&m_recipeRes);
            Logger->debug("RecipeResPublisher::publish: Publish Successful");
            return true;
        }
        Logger->debug("RecipeResPublisher::publish: Publish Fail Due to No Match");
        return false;
    }
    catch (char const* error)
    {
        std::string err(error);
        err = "RecipeResPublisher::publish: " + err;
        Logger->debug(err);
        return false;
    }
    
}

