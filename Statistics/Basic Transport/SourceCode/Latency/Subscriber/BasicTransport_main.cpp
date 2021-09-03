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
 * @file BasicTransport_main.cpp
 *
 */

#include "BasicTransportPublisher.h"
#include "BasicTransportSubscriber.h"

#include <fastrtps/Domain.h>
#include <fastrtps/log/Log.h>

using namespace eprosima;
using namespace fastrtps;
using namespace rtps;

int main(int argc, char** argv)
{
    unsigned int size = 65536;
    std::cout << "Starting "<< std::endl;

    unsigned int index = 0;
    unsigned int preIndex = 0;

    BasicTransportPublisher mypub;
    BasicTransportSubscriber mysub;
    
    if(mypub.init(size))
    {
        if(mysub.init())
        {
            mysub.bindIndex(&index);
            while (true)
            {
                if(index > preIndex)
                {
                    mypub.publish(index);
                    preIndex = index;
                }
            }
        }
    }
    Domain::stopAll();
    Log::Reset();
    return 0;
}
