// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");

// Hpshboss modifys code from eprosima's github example;
// Licensed under the Apache License, Version 2.0 (the "License");

/**
 * @file Publisher.cpp
 *
 */
#include "picturePubSubTypes.h"

#include "Publisher.h"
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/attributes/PublisherAttributes.h>
#include <fastrtps/publisher/Publisher.h>
#include <fastrtps/Domain.h>

#include <thread>
#include <time.h>
#include <vector>

#include <opencv2/opencv.hpp>

#include "picture.h"

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;
using namespace eprosima::fastrtps::types;

// using namespace cv;

PicturePublisher::PicturePublisher()
    : mp_participant(nullptr)
    , mp_publisher(nullptr)
{
}

bool PicturePublisher::init()
{
    std::cout << "Initializing..." << std::endl;
    cv::Mat image = cv::imread("drone.jpg", 1);
    std::vector<unsigned char> buffer;
    cv::imencode(".jpg", image, buffer);
    

    m_picture.index(10);
    m_picture.note("No Comment");
    m_picture.buffer(buffer);
    m_picture.size(buffer.size());

    ParticipantAttributes PParam;
    // PParam.rtps.builtin.discovery_config.discoveryProtocol = DiscoveryProtocol_t::SIMPLE;
    // PParam.rtps.builtin.discovery_config.use_SIMPLE_EndpointDiscoveryProtocol = true;
    // PParam.rtps.builtin.discovery_config.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter = true;
    // PParam.rtps.builtin.discovery_config.m_simpleEDP.use_PublicationWriterANDSubscriptionReader = true;
    // PParam.rtps.builtin.discovery_config.leaseDuration = c_TimeInfinite;
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
    // Wparam.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
    // Wparam.topic.historyQos.depth = 30;
    // Wparam.topic.resourceLimitsQos.max_samples = 5000;
    // Wparam.topic.resourceLimitsQos.allocated_samples = 100;
    // Wparam.times.heartbeatPeriod.seconds = 2;
    // Wparam.times.heartbeatPeriod.nanosec = 200 * 1000 * 1000;
    // Wparam.qos.m_reliability.kind = BEST_EFFORT_RELIABILITY_QOS;
    mp_publisher = Domain::createPublisher(mp_participant, Wparam, (PublisherListener*)&m_listener);
    if (mp_publisher == nullptr)
    {
        return false;
    }
    std::cout << "Initialization Complete" << std::endl;
    return true;

}

PicturePublisher::~PicturePublisher()
{
    Domain::removeParticipant(mp_participant);

    Domain::stopAll();
}

void PicturePublisher::PubListener::onPublicationMatched(
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

void PicturePublisher::PartListener::onParticipantDiscovery(
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

void PicturePublisher::runThread(
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

            if (i == 9){
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

void PicturePublisher::run(
        uint32_t samples,
        uint32_t sleep)
{
    stop = false;
    std::thread thread(&PicturePublisher::runThread, this, samples, sleep);
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

bool PicturePublisher::publish(
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
