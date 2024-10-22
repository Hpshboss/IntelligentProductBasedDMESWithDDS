// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");

// Hpshboss modifys code from eprosima's github example;
// Licensed under the Apache License, Version 2.0 (the "License");

/**
 * @file PicturePublisher.cpp
 *
 */

#include "Publisher.h"
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/attributes/PublisherAttributes.h>
#include <fastrtps/publisher/Publisher.h>
#include <fastrtps/Domain.h>
#include <fastrtps/types/DynamicTypeBuilderFactory.h>
#include <fastrtps/types/DynamicDataFactory.h>
#include <fastrtps/types/DynamicTypeBuilder.h>
#include <fastrtps/types/DynamicTypeBuilderPtr.h>
#include <fastrtps/types/DynamicType.h>

#include <thread>
#include <time.h>
#include <vector>

#include <opencv2/opencv.hpp>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;
using namespace eprosima::fastrtps::types;

// using namespace cv;

PicturePublisher::PicturePublisher()
    : mp_participant(nullptr)
    , mp_publisher(nullptr)
    , m_DynType(DynamicType_ptr(nullptr))
{
}

bool PicturePublisher::init()
{
    cv::Mat image = cv::imread("drone.jpg", 1);
    std::vector<unsigned char> buffer;
    cv::imencode(".jpg", image, buffer);
    
    // Create basic builders
    DynamicTypeBuilder_ptr struct_type_builder(DynamicTypeBuilderFactory::get_instance()->create_struct_builder());

    DynamicType_ptr octet_type(DynamicTypeBuilderFactory::get_instance()->create_byte_type());
    DynamicTypeBuilder_ptr sequence_type_builder(DynamicTypeBuilderFactory::get_instance()->create_sequence_builder(octet_type, 3873715));
    DynamicType_ptr sequence_type = sequence_type_builder->build();
    
    // Add members to the struct. By the way, id must be consecutive starting by zero.
    struct_type_builder->add_member(0, "index", DynamicTypeBuilderFactory::get_instance()->create_uint32_type());
    struct_type_builder->add_member(1, "size", DynamicTypeBuilderFactory::get_instance()->create_uint32_type());
    struct_type_builder->add_member(2, "Picture", sequence_type);
    struct_type_builder->set_name("Picture"); // Need to be same with topic data type
    
    DynamicType_ptr dynType = struct_type_builder->build();
    m_DynType.SetDynamicType(dynType);
    m_DynHello = DynamicDataFactory::get_instance()->create_data(dynType);
    
    m_DynHello->set_uint32_value(0, 0);
    m_DynHello->set_uint32_value(buffer.size(), 1);
    
    MemberId id;
    // std::cout << "init: " << id << std::endl;
    
    DynamicData* sequence_data = m_DynHello->loan_value(2);
    for (int i = 0; i < buffer.size(); i++) {
        if (i == buffer.size() - 1) {
            std::cout << "Total Size: " << i + 1 << std::endl;
        }
        sequence_data->insert_byte_value(buffer[i], id);
    }
    m_DynHello->return_loaned_value(sequence_data);

    ParticipantAttributes PParam;
    PParam.rtps.setName("DynPicture_pub");
    mp_participant = Domain::createParticipant(PParam, (ParticipantListener*)&m_part_list);

    if (mp_participant == nullptr)
    {
        return false;
    }

    //REGISTER THE TYPE
    Domain::registerDynamicType(mp_participant, &m_DynType);

    //CREATE THE PUBLISHER
    PublisherAttributes Wparam;
    Wparam.topic.topicKind = NO_KEY;
    Wparam.topic.topicDataType = "Picture";
    Wparam.topic.topicName = "PictureTopic";

    mp_publisher = Domain::createPublisher(mp_participant, Wparam, (PublisherListener*)&m_listener);
    if (mp_publisher == nullptr)
    {
        return false;
    }

    return true;

}

PicturePublisher::~PicturePublisher()
{
    Domain::removeParticipant(mp_participant);

    DynamicDataFactory::get_instance()->delete_data(m_DynHello);

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
            m_DynHello->get_uint32_value(index, 0);
            std::cout << "runThreading...; \tSample Index: " << index << "; \t";

            uint32_t size;
            m_DynHello->get_uint32_value(size, 1);
            std::cout << "size: " << size << std::endl;
            

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
        m_DynHello->get_uint32_value(index, 0);
        m_DynHello->set_uint32_value(index + 1, 0);

        mp_publisher->write((void*)m_DynHello);
        
        return true;
    }
    return false;
}
