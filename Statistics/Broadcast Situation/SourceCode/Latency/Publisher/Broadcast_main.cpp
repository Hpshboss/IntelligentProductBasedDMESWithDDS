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

#include "BroadcastPublisher.h"
#include "BroadcastSubscriber.h"

#include <fastrtps/Domain.h>
#include <fastrtps/log/Log.h>

#include <thread>
#include <fstream>
#include <chrono>

using namespace eprosima;
using namespace fastrtps;
using namespace rtps;

int main(int argc, char** argv)
{
    unsigned int size = 65536;
    std::cout << "Starting "<< std::endl;
    std::string fileName = "three_sub_single_broadcast_record_latency_" + std::to_string(size) + "bytes.csv";
    std::ofstream myFile(fileName);
    
    try
    {
        unsigned int index = 0;
        BroadcastPublisher mypub;
        BroadcastSubscriber mysub;
        
        if(mypub.init(size))
        {
            if(mysub.init())
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(3000));
                mysub.bindIndex(&index);
                for (int i = 1; i <= 1000; i++)
                {
                    std::cout << "Pub publish " << i;

                    // Get starting timepoint
                    auto start = std::chrono::high_resolution_clock::now();

                    mypub.publish(i);

                    while (index != i * 3) {}

                    // Get ending timepoint
                    auto stop = std::chrono::high_resolution_clock::now();

                    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

                    // Send data to the stream
                    myFile << i << "," << duration.count() << "\n";
                    
                    index = 0;
                    
                    std::cout << ", and receive response" << std::endl;

                    std::this_thread::sleep_for(std::chrono::milliseconds(1500));
                }
            }
        }
    }

    catch (const std::exception & exc)
    {
        std::cout << "Error in main: " << exc.what() << std::endl;
    }

    myFile.close();
    Domain::stopAll();
    Log::Reset();
    return 0;
}
