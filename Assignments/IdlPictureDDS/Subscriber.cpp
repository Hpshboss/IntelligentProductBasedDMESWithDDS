// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");

// Hpshboss modifys code from eprosima's github example;
// Licensed under the Apache License, Version 2.0 (the "License");

/**
 * @file Subscriber.cpp
 *
 */

#include "Subscriber.h"
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/attributes/SubscriberAttributes.h>
#include <fastrtps/subscriber/Subscriber.h>
#include <fastrtps/Domain.h>

#include <vector>
#include <string>
#include <sstream>
#include <iterator>

#include <opencv2/opencv.hpp>

#include <fastrtps/transport/UDPv4TransportDescriptor.h>
#include <fastrtps/transport/TCPv4TransportDescriptor.h>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;
using namespace eprosima::fastrtps::types;

// using namespace cv;

PictureSubscriber::PictureSubscriber()
    : mp_participant(nullptr)
    , mp_subscriber(nullptr)
{
}

struct timespec begin, end;
double elapsed;
std::vector<unsigned char> buffer;

bool PictureSubscriber::init()
{
    ParticipantAttributes PParam;
    // PParam.rtps.builtin.discovery_config.discoveryProtocol = DiscoveryProtocol_t::SIMPLE;
    // PParam.rtps.builtin.discovery_config.use_SIMPLE_EndpointDiscoveryProtocol = true;
    // PParam.rtps.builtin.discovery_config.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter = true;
    // PParam.rtps.builtin.discovery_config.m_simpleEDP.use_PublicationWriterANDSubscriptionReader = true;
    // PParam.rtps.builtin.discovery_config.leaseDuration = c_TimeInfinite;
    int32_t kind = LOCATOR_KIND_TCPv4;

    Locator_t initial_peer_locator;
    initial_peer_locator.kind = kind;
    IPLocator::setIPv4(initial_peer_locator, "192.168.1.101");
    initial_peer_locator.port = 5100;
    PParam.rtps.builtin.initialPeersList.push_back(initial_peer_locator); // Publisher's channel

    PParam.rtps.useBuiltinTransports = false;
    std::shared_ptr<TCPv4TransportDescriptor> descriptor = std::make_shared<TCPv4TransportDescriptor>();
    descriptor->sendBufferSize = 8912896; // 8.5Mb
    descriptor->receiveBufferSize = 8912896; // 8.5Mb
    PParam.rtps.userTransports.push_back(descriptor);
    PParam.rtps.setName("Participant_sub");
    mp_participant = Domain::createParticipant(PParam, (ParticipantListener*)&m_part_list);

    //REGISTER THE TYPE
    Domain::registerType(mp_participant, &m_type);

    //CREATE THE SUBSCRIBER
    SubscriberAttributes Rparam;
    Rparam.topic.topicKind = NO_KEY;
    Rparam.topic.topicDataType = "picture";
    Rparam.topic.topicName = "pictureTopic";
    // Rparam.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
    // Rparam.topic.historyQos.depth = 30;
    // Rparam.topic.resourceLimitsQos.max_samples = 5000;
    // Rparam.topic.resourceLimitsQos.allocated_samples = 100;
    Rparam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
    // Rparam.qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
    mp_subscriber = Domain::createSubscriber(mp_participant, Rparam, (SubscriberListener*)&m_listener);

    if (mp_subscriber == nullptr)
    {
        return false;
    }


    return true;
}

PictureSubscriber::~PictureSubscriber()
{
    Domain::removeParticipant(mp_participant);

    Domain::stopAll();
}

void PictureSubscriber::SubListener::onSubscriptionMatched(
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

void PictureSubscriber::PartListener::onParticipantDiscovery(
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

void PictureSubscriber::SubListener::onNewDataMessage(
        Subscriber* sub)
{
    if (sub->takeNextData(&m_picture, &m_info))
    {
        if (m_info.sampleKind == ALIVE)
        {
            this->n_samples++;
            
            // Print your structure data here.
            uint32_t index;
            index = m_picture.index();
            std::cout << "index: " << index <<  "; \t";

            uint32_t size;
            size = m_picture.size();
            std::cout << "Buffer Size: " << size <<  "; \t";

            std::cout << "Note: " << m_picture.note() << "; \t";

            buffer = m_picture.buffer();

            cv::Mat imageDecoded = cv::imdecode(buffer, 1);
            cv::imwrite(std::to_string(index) + "_droneNew.jpg", imageDecoded);
            std::cout << "Store Complete" << std::endl;
        }
    }
}

void PictureSubscriber::run()
{
    std::cout << "Subscriber running. Please press enter to stop the Subscriber" << std::endl;
    std::cin.ignore();
}

void PictureSubscriber::run(
        uint32_t number)
{
    std::cout << "Subscriber running until " << number << "samples have been received" << std::endl;
    while (number > this->m_listener.n_samples)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}
