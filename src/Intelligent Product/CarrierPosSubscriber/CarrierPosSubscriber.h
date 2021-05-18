// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");

// Hpshboss modifys code from eprosima's github example;
// Licensed under the Apache License, Version 2.0 (the "License");

/**
 * @file Publisher.h
 *
 */

#ifndef CARRIERPOSPUBLISHER_H_
#define CARRIERPOSPUBLISHER_H_

#include "carrierPosPubSubTypes.h"
#include "carrierPos.h"

#include <fastrtps/fastrtps_fwd.h>

#include <fastrtps/attributes/SubscriberAttributes.h>
#include <fastrtps/subscriber/SubscriberListener.h>

#include <fastrtps/subscriber/SampleInfo.h>

#include <fastrtps/participant/ParticipantListener.h>
#include <fastrtps/participant/Participant.h>

#include <fastrtps/TopicDataType.h>

// #include <spglog/spdlog.h>
// #include <fastrtps/log/Log.h>
#include <opc/common/logger.h>

#include <string>
#include <map>
#include <boost/any.hpp>


class CarrierPosSubscriber
{
    public:

        CarrierPosSubscriber(std::shared_ptr<spdlog::logger> logger);

        virtual ~CarrierPosSubscriber();

        // initial
        bool init();
        
    private:
        carrierPosPubSubType m_type;
        
        eprosima::fastrtps::Participant* mp_participant;

        eprosima::fastrtps::Subscriber* mp_subscriber;


        class SubListener : public eprosima::fastrtps::SubscriberListener
        {
            public:
                SubListener()
                    : n_matched(0)
                {}

                ~SubListener() override {}

                void onSubscriptionMatched(
                        eprosima::fastrtps::Subscriber* sub,
                        eprosima::fastrtps::rtps::MatchingInfo& info) override;

                void onNewDataMessage(
                        eprosima::fastrtps::Subscriber* sub) override;

                carrierPos m_carrierPos;

                eprosima::fastrtps::SampleInfo_t m_info;

                int n_matched;

                Common::Logger::SharedPtr LLogger;

        } m_listener;

        class PartListener : public eprosima::fastrtps::ParticipantListener
        {
            void onParticipantDiscovery(
                    eprosima::fastrtps::Participant* participant,
                    eprosima::fastrtps::rtps::ParticipantDiscoveryInfo&& info) override;
        } m_part_list;
        

        Common::Logger::SharedPtr Logger;
};



#endif /* ORDERINFOPUBLISHER_H_ */