// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");

// Hpshboss modifys code from eprosima's github example;
// Licensed under the Apache License, Version 2.0 (the "License");

/**
 * @file Publisher.cpp
 *
 */
#include "recipeInfoPubSubTypes.h"

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

#include "recipeInfo.h"

#include "RecipeInfoPublisher.h"

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;
using namespace eprosima::fastrtps::types;

// using namespace cv;

RecipeInfoPublisher::RecipeInfoPublisher(std::shared_ptr<spdlog::logger> logger)
    : mp_participant(nullptr)
    , mp_publisher(nullptr)
    , Logger(logger)
{
}


RecipeInfoPublisher::~RecipeInfoPublisher()
{
    Domain::removeParticipant(mp_participant);

    Domain::stopAll();
}


bool RecipeInfoPublisher::init()
{
    ParticipantAttributes PParam;
    PParam.rtps.setName("Participant_pub_recipeInfo");

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
    Wparam.topic.topicDataType = "recipeInfo";
    Wparam.topic.topicName = "recipeInfo";
    Wparam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;

    mp_publisher = Domain::createPublisher(mp_participant, Wparam, (PublisherListener*)&m_listener);

    if (mp_publisher == nullptr)
    {
        return false;
    }

    return true;
}

void RecipeInfoPublisher::PubListener::onPublicationMatched(
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

void RecipeInfoPublisher::PartListener::onParticipantDiscovery(
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

bool RecipeInfoPublisher::publish(unsigned int orderNumber, unsigned int orderPosition, std::string workPlan, std::string note)
{
    Logger->debug("RecipeInfoPublisher::publish");
    try
    {
        if (m_listener.n_matched > 0) 
        {
            m_recipeInfo.orderNumber(orderNumber);
            m_recipeInfo.orderPosition(orderPosition);
            m_recipeInfo.workPlan(workPlan);
            m_recipeInfo.note(note);
            mp_publisher->write(&m_recipeInfo);
            Logger->debug("RecipeInfoPublisher::publish: Publish Successful");
            return true;
        }
        Logger->debug("RecipeInfoPublisher::publish: Publish Fail Due to No Match");
        return false;
    }
    catch (char const* error)
    {
        std::string err(error);
        err = "RecipeInfoPublisher::publish: " + err;
        Logger->debug(err);
        return false;
    }
}

