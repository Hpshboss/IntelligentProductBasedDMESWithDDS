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


class OrderInfoSubscriber
{
    public:

        OrderInfoSubscriber(std::shared_ptr<spdlog::logger> logger);

        virtual ~OrderInfoSubscriber();

        // initial
        bool init();
        
    private:
    
        orderInfoPubSubType m_type;
        
        eprosima::fastrtps::Participant* mp_participant;

        eprosima::fastrtps::Subscriber* mp_subscriber;
        
        Common::Logger::SharedPtr Logger;

        class SubListener : public eprosima::fastrtps::SubscriberListener
        {
            public:
                SubListener(std::shared_ptr<spdlog::logger> logger)
                    : n_matched(0)
                    , Logger(logger)
                {}

                ~SubListener() override {}

                void onSubscriptionMatched(
                        eprosima::fastrtps::Subscriber* sub,
                        eprosima::fastrtps::rtps::MatchingInfo& info) override;

                void onNewDataMessage(
                        eprosima::fastrtps::Subscriber* sub) override;

                orderInfo m_orderInfo;

                eprosima::fastrtps::SampleInfo_t m_info;

                int n_matched;

            private: 
                Common::Logger::SharedPtr Logger;

        };

        class PartListener : public eprosima::fastrtps::ParticipantListener
        {
            void onParticipantDiscovery(
                    eprosima::fastrtps::Participant* participant,
                    eprosima::fastrtps::rtps::ParticipantDiscoveryInfo&& info) override;
        } m_part_list;
        
        SubListener m_listener;
};



#endif /* ORDERINFOPUBLISHER_H_ */