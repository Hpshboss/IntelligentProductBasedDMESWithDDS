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

#include "assignedOpRes.h"

#include "AssignedOpResPublisher.h"

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;
using namespace eprosima::fastrtps::types;

// using namespace cv;

AssignedOpResPublisher::AssignedOpResPublisher(std::shared_ptr<spdlog::logger> logger)
    : mp_participant(nullptr)
    , mp_publisher(nullptr)
    , Logger(logger)
{
}


AssignedOpResPublisher::~AssignedOpResPublisher()
{
    Domain::removeParticipant(mp_participant);

    Domain::stopAll();
}


bool AssignedOpResPublisher::init()
{
    ParticipantAttributes PParam;
    PParam.rtps.setName("Participant_pub_assignedOpRes");

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
    Wparam.topic.topicDataType = "assignedOpRes";
    Wparam.topic.topicName = "assignedOpRes";
    Wparam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;

    mp_publisher = Domain::createPublisher(mp_participant, Wparam, (PublisherListener*)&m_listener);

    if (mp_publisher == nullptr)
    {
        return false;
    }

    return true;
}

void AssignedOpResPublisher::PubListener::onPublicationMatched(
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

void AssignedOpResPublisher::PartListener::onParticipantDiscovery(
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

bool AssignedOpResPublisher::publish(unsigned int resourceId, unsigned int portId, std::string GUID, unsigned int carrierId, std::string operationInfo, std::string result, std::string note)
{
    Logger->debug("AssignedOpResPublisher::publish");
    try
    {
        if (m_listener.n_matched > 0) 
        {
            m_assignedOpRes.resourceId(resourceId);
            m_assignedOpRes.portId(portId);
            m_assignedOpRes.GUID(GUID);
            m_assignedOpRes.carrierId(carrierId);
            m_assignedOpRes.operationInfo(operationInfo);
            m_assignedOpRes.result(result);
            m_assignedOpRes.note(note);
            mp_publisher->write(&m_assignedOpRes);
            Logger->debug("AssignedOpResPublisher::publish: Publish Successful");
            return true;
        }
        Logger->debug("AssignedOpResPublisher::publish: Publish Fail Due to No Match");
        return false;
    }
    catch (char const* error)
    {
        std::string err(error);
        err = "AssignedOpResPublisher::publish: " + err;
        Logger->debug(err);
        return false;
    }
}

