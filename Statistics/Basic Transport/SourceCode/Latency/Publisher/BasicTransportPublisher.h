// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/**
 * @file BasicTransportPublisher.h
 *
 */

#ifndef BASICTRANSPORTPUBLISHER_H_
#define BASICTRANSPORTPUBLISHER_H_

#include "BasicTransportPubSubTypes.h"

#include <fastrtps/fastrtps_fwd.h>
#include <fastrtps/attributes/PublisherAttributes.h>
#include <fastrtps/publisher/PublisherListener.h>


#include "BasicTransport.h"

class BasicTransportPublisher {
public:
	BasicTransportPublisher();
	virtual ~BasicTransportPublisher();
	//!Initialize
	bool init(unsigned int size);
	//!Publish a sample
	bool publish(unsigned int index);
private:
	BasicTransport m_BasicTransport;
	eprosima::fastrtps::Participant* mp_participant;
	eprosima::fastrtps::Publisher* mp_publisher;
	bool stop;
	class PubListener:public eprosima::fastrtps::PublisherListener
	{
	public:
		PubListener():n_matched(0),firstConnected(false){};
		~PubListener(){};
		void onPublicationMatched(eprosima::fastrtps::Publisher* pub, eprosima::fastrtps::rtps::MatchingInfo& info);
		int n_matched;
        bool firstConnected;
	}m_listener;
	void runThread(uint32_t number, uint32_t sleep);
	BasicTransportPubSubType m_type;
};



#endif /* BASICTRANSPORTPUBLISHER_H_ */
