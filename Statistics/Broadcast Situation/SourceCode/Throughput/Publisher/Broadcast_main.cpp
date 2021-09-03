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

#include <fastrtps/Domain.h>
#include <fastrtps/log/Log.h>

#include <thread>
#include <fstream>
#include <chrono>
#include <math.h>

using namespace eprosima;
using namespace fastrtps;
using namespace rtps;

int main(int argc, char** argv)
{
    unsigned int size = (unsigned int) pow(2.0, 23.0);
    std::cout << "Size: " << size << " Bytes" << std::endl;
    
    try
    {
        unsigned int index = 0;
        BroadcastPublisher mypub;
        
        if(mypub.init(size))
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(5000));
            unsigned int i = 10;
            while (true)
            {
                mypub.publish(i);
            }
        }
    }

    catch (const std::exception & exc)
    {
        std::cout << "Error in main: " << exc.what() << std::endl;
    }

    Domain::stopAll();
    Log::Reset();
    return 0;
}
