// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");

// Hpshboss modifys code from eprosima's github example;
// Licensed under the Apache License, Version 2.0 (the "License");

/**
 * @file Subscriber.h
 *
 */

#ifndef PICTURESUBSCRIBER_H_
#define PICTURESUBSCRIBER_H_

#include "picturePubSubTypes.h"

#include <fastrtps/fastrtps_fwd.h>
#include <fastrtps/attributes/SubscriberAttributes.h>
#include <fastrtps/subscriber/SubscriberListener.h>
#include <fastrtps/subscriber/SampleInfo.h>
#include <fastrtps/participant/ParticipantListener.h>
#include <fastrtps/participant/Participant.h>

#include "picture.h"

class PictureSubscriber
{
    public:

        PictureSubscriber();

        virtual ~PictureSubscriber();
    
        //!Initialize the subscriber
        bool init();

        //!RUN the subscriber
        void run();

        //!Run the subscriber until number samples have been recevied.
        void run(
                uint32_t number);

    private:

        eprosima::fastrtps::Participant* mp_participant;

        eprosima::fastrtps::Subscriber* mp_subscriber;

    public:

        class SubListener : public eprosima::fastrtps::SubscriberListener
        {
            public:
                SubListener()
                    : n_matched(0)
                    , n_samples(0)
                {}

                ~SubListener() override {}

                void onSubscriptionMatched(
                        eprosima::fastrtps::Subscriber* sub,
                        eprosima::fastrtps::rtps::MatchingInfo& info) override;

                void onNewDataMessage(
                        eprosima::fastrtps::Subscriber* sub) override;

                picture m_picture;

                eprosima::fastrtps::SampleInfo_t m_info;

                int n_matched;

                uint32_t n_samples;
        } m_listener;

        class PartListener : public eprosima::fastrtps::ParticipantListener
        {
            void onParticipantDiscovery(
                    eprosima::fastrtps::Participant* p,
                    eprosima::fastrtps::rtps::ParticipantDiscoveryInfo&& info) override;
        } m_part_list;
    
    private:
        picturePubSubType m_type;
};

#endif /* HELLOWORLDSUBSCRIBER_H_ */
