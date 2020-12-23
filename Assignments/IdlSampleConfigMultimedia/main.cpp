// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");

// Hpshboss modifys code from eprosima's github example;
// Licensed under the Apache License, Version 2.0 (the "License");

/**
 * @file main.cpp
 *
 */

#include "Publisher.h"
#include "Subscriber.h"

#include <fastrtps/Domain.h>
#include <fastrtps/log/Log.h>

#include <fstream>

using namespace eprosima;
using namespace fastrtps;
using namespace rtps;

int type = 1;
int count = 50;
long sleep = 500;


void usage()
{
    std::cout << "USAGE:" << std::endl;
    std::cout << "DynamicPicture [role] [-c count] [-s sleep]" << std::endl;
    std::cout << "role: Can be 'publisher' (default) or 'subscriber'." << std::endl;
    std::cout << "count: Number of samples sent by a publisher. Without effect on subcribers." << std::endl;
    std::cout << "sleep: Time between samples sent by a publisher in millisecond. Without effect on subcribers." << std::endl;
}

bool parseArgs(int argc, char** argv)
{
    bool roleDefined = false;
    try
    {
        for (int i = 1; i < argc; ++i)
        {
            if(strcmp(argv[i],"publisher")==0)
            {
                if (!roleDefined)
                {
                    type = 1;
                    roleDefined = true;
                }
                else
                {
                    std::cout << "role defined twice" << std::endl;
                    Log::Reset();
                    return false;
                }
            }
            else if(strcmp(argv[i],"subscriber")==0)
            {
                if (!roleDefined)
                {
                    type = 2;
                    roleDefined = true;
                }
                else
                {
                    std::cout << "role defined twice" << std::endl;
                    Log::Reset();
                    return false;
                }
            }
            else if(strcmp(argv[i],"-c")==0)
            {
                if (argc <= i+1)
                {
                    std::cout << "count expected" << std::endl;
                    Log::Reset();
                    return false;
                }
                count = atoi(argv[++i]);
            }
            else if(strcmp(argv[i],"-s")==0)
            {
                if (argc <= i+1)
                {
                    std::cout << "sleep expected" << std::endl;
                    Log::Reset();
                    return false;
                }
                sleep = atoi(argv[++i]);
            }
        }
    }
    catch(std::exception&)
    {
        usage();
        return false;
    }

    return true;
}

int main(int argc, char** argv)
{
    //Log::SetVerbosity(Log::Kind::Info);
    //std::regex filter("RTPS_HISTORY");
    //std::regex filter("RTPS_EDP");
    //Log::SetCategoryFilter(filter);
    std::cout << "Starting "<< std::endl;

    
    
    if(!parseArgs(argc, argv))
    {
        usage();
        return 0;
    }

    //Log::SetVerbosity(Log::Kind::Info);
    switch(type)
    {
        case 1:
            {
                SamplePublisher mypub;
                if(mypub.init())
                {
                    mypub.run(count, sleep);
                }
                break;
            }
        case 2:
            {
                SampleSubscriber mysub;
                if(mysub.init())
                {
                    mysub.run();
                }
                break;
            }
    }
    Domain::stopAll();
    Log::Reset();
    
    return 0;
}
