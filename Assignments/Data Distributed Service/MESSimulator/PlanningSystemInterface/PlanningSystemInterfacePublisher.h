// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");

// Hpshboss modifys code from eprosima's github example;
// Licensed under the Apache License, Version 2.0 (the "License");

/**
 * @file Publisher.h
 *
 */

#ifndef ORDERINFOPUBLISHER_H_
#define ORDERINFOPUBLISHER_H_

#include "orderInfoPubSubTypes.h"
#include "orderInfo.h"

#include <fastrtps/fastrtps_fwd.h>

#include <fastrtps/attributes/PublisherAttributes.h>
#include <fastrtps/publisher/PublisherListener.h>

#include <fastrtps/subscriber/SampleInfo.h>

#include <fastrtps/participant/ParticipantListener.h>
#include <fastrtps/participant/Participant.h>



class PlanningSystemInterfacePublisher
{
    public:

        PlanningSystemInterfacePublisher();

        virtual ~PlanningSystemInterfacePublisher();

        //!Initialize
        bool init();

        //!Publish a sample
        bool publish(
                bool waitForListener = true);

        //!Run for number samples
        void run(
                uint32_t number,
                uint32_t sleep);

    private:
        orderInfo m_orderInfo;

        eprosima::fastrtps::Participant* mp_participant;

        eprosima::fastrtps::Publisher* mp_publisher;

        bool stop;

        class PubListener : public eprosima::fastrtps::PublisherListener
        {
            public:
                PubListener()
                    : n_matched(0)
                    , firstConnected(false)
                {}

                ~PubListener() override {}

                void onPublicationMatched(
                        eprosima::fastrtps::Publisher* pub,
                        eprosima::fastrtps::rtps::MatchingInfo& info) override;

                int n_matched;

                bool firstConnected;

        } m_listener;

        class PartListener : public eprosima::fastrtps::ParticipantListener
        {
            void onParticipantDiscovery(
                    eprosima::fastrtps::Participant* participant,
                    eprosima::fastrtps::rtps::ParticipantDiscoveryInfo&& info) override;
        } m_part_list;

        void runThread(
                uint32_t number,
                uint32_t sleep);

        orderInfoPubSubType m_type;
};



#endif /* ORDERINFOPUBLISHER_H_ */