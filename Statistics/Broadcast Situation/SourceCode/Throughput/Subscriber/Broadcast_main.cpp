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
 * @file Broadcast_main.cpp
 *
 */

#include "BroadcastSubscriber.h"

#include <fastrtps/Domain.h>
#include <fastrtps/log/Log.h>

#include <chrono>

using namespace eprosima;
using namespace fastrtps;
using namespace rtps;

int main(int argc, char** argv)
{
    unsigned int size = (unsigned int) pow(2.0, 23.0);
    std::cout << "Starting "<< std::endl;
    std::cout << "Size: " << size << " Bytes" <<std::endl;

    unsigned int count = 0;
    unsigned int totalCount = 0;

    BroadcastSubscriber mysub;
    
    if(mysub.init())
    {
        mysub.bindCount(&count);
        
        while (count == 0) {}

        // Get starting timepoint
        std::cout << "Starting clock"<< std::endl;
        auto start = std::chrono::high_resolution_clock::now();

        std::this_thread::sleep_for(std::chrono::milliseconds(90000));

        while (true) 
        {
            // Get ending timepoint
            auto stop = std::chrono::high_resolution_clock::now();

            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

            // Send data to the stream
            if(duration.count() > 100000000)
            {
                std::cout << count << std::endl;
                break;
            }
        }
    }

    Domain::stopAll();
    Log::Reset();
    return 0;
}
