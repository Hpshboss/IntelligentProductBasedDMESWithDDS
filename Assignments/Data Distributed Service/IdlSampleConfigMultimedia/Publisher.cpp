// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");

// Hpshboss modifys code from eprosima's github example;
// Licensed under the Apache License, Version 2.0 (the "License");

/**
 * @file Publisher.cpp
 *
 */
#include "samplePubSubTypes.h"

#include "Publisher.h"
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/attributes/PublisherAttributes.h>
#include <fastrtps/publisher/Publisher.h>
#include <fastrtps/Domain.h>

#include <thread>
#include <time.h>
#include <vector>

#include "sample.h"

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;
using namespace eprosima::fastrtps::types;

// using namespace cv;

SamplePublisher::SamplePublisher()
    : mp_participant(nullptr)
    , mp_publisher(nullptr)
{
}

bool SamplePublisher::init()
{
    m_sample.index(10);
    m_sample.key_value(100);

    ParticipantAttributes PParam;
    PParam.rtps.builtin.discovery_config.discoveryProtocol = DiscoveryProtocol_t::SIMPLE;
    PParam.rtps.builtin.discovery_config.use_SIMPLE_EndpointDiscoveryProtocol = true;
    PParam.rtps.builtin.discovery_config.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter = true;
    PParam.rtps.builtin.discovery_config.m_simpleEDP.use_PublicationWriterANDSubscriptionReader = true;
    PParam.rtps.builtin.discovery_config.leaseDuration = c_TimeInfinite;
    PParam.rtps.setName("Participant_pub");
    mp_participant = Domain::createParticipant(PParam);

    if (mp_participant == nullptr)
    {
        return false;
    }

    //REGISTER THE TYPE
    Domain::registerType(mp_participant, &m_type);

    //CREATE THE PUBLISHER
    PublisherAttributes Wparam;
    Wparam.topic.topicKind = NO_KEY;
    Wparam.topic.topicDataType = "sample";
    Wparam.topic.topicName = "sampleConfigTopic";
    Wparam.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
    Wparam.topic.historyQos.depth = 30;
    Wparam.topic.resourceLimitsQos.max_samples = 50;
    Wparam.topic.resourceLimitsQos.allocated_samples = 20;
    Wparam.times.heartbeatPeriod.seconds = 2;
    Wparam.times.heartbeatPeriod.nanosec = 200 * 1000 * 1000;
    Wparam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
    mp_publisher = Domain::createPublisher(mp_participant, Wparam, (PublisherListener*)&m_listener);
    if (mp_publisher == nullptr)
    {
        return false;
    }

    return true;

}

SamplePublisher::~SamplePublisher()
{
    Domain::removeParticipant(mp_participant);

    Domain::stopAll();
}

void SamplePublisher::PubListener::onPublicationMatched(
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

void SamplePublisher::PartListener::onParticipantDiscovery(
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

void SamplePublisher::runThread(
        uint32_t samples,
        uint32_t sleep)
{
    uint32_t i = 0;

    while (!stop && (i < samples || samples == 0))
    {
        if (publish(samples != 0))
        {
            uint32_t index;
            index = m_sample.index();
            std::cout << "runThreading...; \tSample Index: " << index << "; \t";

            uint32_t key_value;
            key_value = m_sample.key_value();
            std::cout << "key value: " << key_value << std::endl;
            

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

void SamplePublisher::run(
        uint32_t samples,
        uint32_t sleep)
{
    stop = false;
    std::thread thread(&SamplePublisher::runThread, this, samples, sleep);
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

bool SamplePublisher::publish(
        bool waitForListener)
{
    // std::cout << "m_listener.n_matched: " << m_listener.n_matched << std::endl;
    if (m_listener.firstConnected || !waitForListener || m_listener.n_matched > 0)
    {
        uint32_t index;
        index = m_sample.index();
        m_sample.index(index + 1);

        mp_publisher->write(&m_sample);
        
        return true;
    }
    return false;
}
