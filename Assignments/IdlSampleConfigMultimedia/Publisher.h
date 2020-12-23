// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");

// Hpshboss modifys code from eprosima's github example;
// Licensed under the Apache License, Version 2.0 (the "License");

/**
 * @file Publisher.h
 *
 */

#ifndef PICTUREPUBLISHER_H_
#define PICTUREPUBLISHER_H_

#include "samplePubSubTypes.h"

#include <fastrtps/fastrtps_fwd.h>
#include <fastrtps/attributes/PublisherAttributes.h>
#include <fastrtps/publisher/PublisherListener.h>
#include <fastrtps/participant/ParticipantListener.h>
#include <fastrtps/participant/Participant.h>

#include "sample.h"

class SamplePublisher
{
    public:

        SamplePublisher();

        virtual ~SamplePublisher();

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
        sample m_sample;

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

        samplePubSubType m_type;
};



#endif /* PICTUREPUBLISHER_H_ */
