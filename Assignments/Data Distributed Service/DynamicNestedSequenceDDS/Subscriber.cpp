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
#include <fastrtps/types/DynamicTypeBuilderFactory.h>
#include <fastrtps/types/DynamicDataFactory.h>
#include <fastrtps/types/DynamicTypeBuilder.h>
#include <fastrtps/types/DynamicTypeBuilderPtr.h>
#include <fastrtps/types/DynamicType.h>

#include <vector>
#include <string>
#include <sstream>
#include <iterator>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;
using namespace eprosima::fastrtps::types;

PictureSubscriber::PictureSubscriber()
    : mp_participant(nullptr)
    , mp_subscriber(nullptr)
    , m_DynType(DynamicType_ptr(nullptr))
{
}

struct timespec begin, end;
unsigned int countSample = 0;
double elapsed;

bool PictureSubscriber::init()
{

    ParticipantAttributes PParam;
    PParam.rtps.setName("DynPicture_sub");
    mp_participant = Domain::createParticipant(PParam, (ParticipantListener*)&m_part_list);
    if (mp_participant == nullptr)
    {
        return false;
    }

    // Create basic builders
    DynamicTypeBuilder_ptr struct_type_builder(DynamicTypeBuilderFactory::get_instance()->create_struct_builder());

    DynamicTypeBuilder_ptr octet_builder(DynamicTypeBuilderFactory::get_instance()->create_byte_builder());
    DynamicTypeBuilder_ptr sequence_type_builder(DynamicTypeBuilderFactory::get_instance()->create_sequence_builder(octet_builder.get(), 5));
    DynamicType_ptr sequence_type = sequence_type_builder->build();

    // Add members to the struct.
    struct_type_builder->add_member(0, "index", DynamicTypeBuilderFactory::get_instance()->create_uint32_type());
    struct_type_builder->add_member(1, "Picture", sequence_type);
    struct_type_builder->set_name("Picture");
    
    DynamicType_ptr dynType = struct_type_builder->build();
    m_DynType.SetDynamicType(dynType);
    m_listener.m_DynHello = DynamicDataFactory::get_instance()->create_data(dynType);

    //REGISTER THE TYPE
    Domain::registerDynamicType(mp_participant, &m_DynType);

    //CREATE THE SUBSCRIBER
    SubscriberAttributes Rparam;
    Rparam.topic.topicKind = NO_KEY;
    Rparam.topic.topicDataType = "Picture";
    Rparam.topic.topicName = "PictureTopic";

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

    DynamicDataFactory::get_instance()->delete_data(m_listener.m_DynHello);

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
        clock_gettime(CLOCK_MONOTONIC, &begin);
    }
    else
    {
        n_matched--;
        clock_gettime(CLOCK_MONOTONIC, &end);
        long seconds = end.tv_sec - begin.tv_sec;
        long nanoseconds = end.tv_nsec - begin.tv_nsec;
        elapsed = double(seconds) + double(nanoseconds)*1e-9;
        std::cout << "Elapsed Time: " << elapsed << std::endl;
        std::cout << "Samples: " << countSample << " of 50" << std::endl;
        countSample = 0;
        elapsed = 0.0;
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
    if (sub->takeNextData((void*)m_DynHello, &m_info))
    {
        if (m_info.sampleKind == ALIVE)
        {
            this->n_samples++;
            countSample++;
            
            // Print your structure data here.
            uint32_t index;
            m_DynHello->get_uint32_value(index, 0);
            std::cout << "index: " << index << ";\t";
            
        
            DynamicType_ptr octet_type_temp(DynamicTypeBuilderFactory::get_instance()->create_byte_type());
            DynamicTypeBuilder_ptr sequence_type_builder_temp(DynamicTypeBuilderFactory::get_instance()->create_sequence_builder(octet_type_temp, 5));
            DynamicType_ptr sequence_type_temp = sequence_type_builder_temp->build();
            DynamicData* sequence_data_temp = m_DynHello->loan_value(1);
            std::cout << "sequence: {" << sequence_data_temp->get_byte_value(0) << ", ";
            std::cout << sequence_data_temp->get_byte_value(1) << ", ";
            std::cout << sequence_data_temp->get_byte_value(2) << ", ";
            std::cout << sequence_data_temp->get_byte_value(3) << ", ";
            std::cout << sequence_data_temp->get_byte_value(4) << "}" << std::endl;
            m_DynHello->return_loaned_value(sequence_data_temp);
            
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
