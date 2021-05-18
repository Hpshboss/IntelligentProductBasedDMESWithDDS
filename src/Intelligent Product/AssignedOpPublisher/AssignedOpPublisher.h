// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");

// Hpshboss modifys code from eprosima's github example;
// Licensed under the Apache License, Version 2.0 (the "License");

/**
 * @file Publisher.h
 *
 */

#ifndef ASSIGNEDOPPUBLISHER_H_
#define ASSIGNEDOPPUBLISHER_H_

#include "assignedOpPubSubTypes.h"
#include "assignedOp.h"

#include <fastrtps/fastrtps_fwd.h>

#include <fastrtps/attributes/PublisherAttributes.h>
#include <fastrtps/publisher/PublisherListener.h>


#include <fastrtps/participant/ParticipantListener.h>
#include <fastrtps/participant/Participant.h>

#include <fastrtps/TopicDataType.h>

// #include <spglog/spdlog.h>
// #include <fastrtps/log/Log.h>
#include <opc/common/logger.h>

#include <string>
#include <map>
#include <boost/any.hpp>


class AssignedOpPublisher
{
    public:

        AssignedOpPublisher(std::shared_ptr<spdlog::logger> logger);

        virtual ~AssignedOpPublisher();

        // initial
        bool init();

        bool publish(unsigned int resourceId, unsigned int portId, unsigned int orderNumber, unsigned int orderPosition, std::string operationInfo, std::string note);
        
    private:
        assignedOp m_assignedOp;

        assignedOpPubSubType m_type;
        
        eprosima::fastrtps::Participant* mp_participant;

        eprosima::fastrtps::Publisher* mp_publisher;


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
        

        Common::Logger::SharedPtr Logger;
};



#endif /* ORDERINFOPUBLISHER_H_ */