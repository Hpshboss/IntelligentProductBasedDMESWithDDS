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

#include "Publisher.h"
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/attributes/PublisherAttributes.h>
#include <fastrtps/publisher/Publisher.h>
#include <fastrtps/Domain.h>

#include <thread>
#include <time.h>
#include <vector>

#include <opencv2/opencv.hpp>

#include <fastrtps/transport/UDPv4TransportDescriptor.h>
#include <fastrtps/transport/TCPv4TransportDescriptor.h>

#include "picture.h"

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;
using namespace eprosima::fastrtps::types;

// using namespace cv;

PlanningSystemInterfacePublisher::PlanningSystemInterfacePublisher()
    : mp_participant(nullptr)
    , mp_publisher(nullptr)
{
}

bool PlanningSystemInterfacePublisher::init()
{
    ParticipantAttributes PParam;
    
    PParam.rtps.useBuiltinTransports = false;

    std::shared_ptr<TCPv4TransportDescriptor> descriptor = std::make_shared<TCPv4TransportDescriptor>();
    descriptor->sendBufferSize = 6000000; // 6Mb
    descriptor->receiveBufferSize = 6000000; // 6Mb
    descriptor->add_listener_port(5100);
    PParam.rtps.userTransports.push_back(descriptor);
    PParam.rtps.setName("Participant_pub");
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
    Wparam.topic.topicDataType = "picture";
    Wparam.topic.topicName = "pictureTopic";

    Wparam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
    mp_publisher = Domain::createPublisher(mp_participant, Wparam, (PublisherListener*)&m_listener);
    if (mp_publisher == nullptr)
    {
        return false;
    }

    return true;
}

PlanningSystemInterfacePublisher::~PlanningSystemInterfacePublisher()
{
    Domain::removeParticipant(mp_participant);

    Domain::stopAll();
}

void PlanningSystemInterfacePublisher::PubListener::onPublicationMatched(
        Publisher* /*pub*/,
        MatchingInfo& info)
{
    if (info.status == MATCHED_MATCHING)
    {
        n_matched++;
        firstConnected = true;
        std::cout << "Publisher matched" << std::endl;
    }
    else
    {
        n_matched--;
        std::cout << "Publisher unmatched" << std::endl;
    }
}

void PlanningSystemInterfacePublisher::PartListener::onParticipantDiscovery(
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

void PlanningSystemInterfacePublisher::runThread(
        uint32_t samples,
        uint32_t sleep)
{
    uint32_t i = 0;

    while (!stop && (i < samples || samples == 0))
    {
        if (publish(samples != 0))
        {
            uint32_t index;
            index = m_picture.index();
            std::cout << "runThreading...; \tPicture Index: " << index << "; \t";

            uint32_t size;
            size = m_picture.size();
            std::cout << "Buffer Size: " << size << std::endl;
            
            std::cout << "Note: " << m_picture.note() << std::endl;

            if (i == 19){
                std::cout << "Structure message" << " with index: " << i + 1 << " SENT" << std::endl;
                // Avoid unmatched condition impact subscriber receiving message
                std::cout << "Wait within twenty second..." << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(10000));
            }
            ++i;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(sleep));
    }
}

void PlanningSystemInterfacePublisher::run(
        uint32_t samples,
        uint32_t sleep)
{
    stop = false;
    std::thread thread(&PlanningSystemInterfacePublisher::runThread, this, samples, sleep);
    if (samples == 0)
    {
        std::cout << "Publisher running. Please press enter to stop the Publisher at any time." << std::endl;
        std::cin.ignore();
        stop = true;
    }
    else
    {
        std::cout << "Publisher running " << samples << " samples." << std::endl;
    }
    thread.join();
}

bool PlanningSystemInterfacePublisher::publish(
        bool waitForListener)
{
    // std::cout << "m_listener.n_matched: " << m_listener.n_matched << std::endl;
    if (m_listener.firstConnected || !waitForListener || m_listener.n_matched > 0)
    {
        uint32_t index;
        index = m_picture.index();
        m_picture.index(index + 1);

        mp_publisher->write(&m_picture);
        
        return true;
    }
    return false;
}