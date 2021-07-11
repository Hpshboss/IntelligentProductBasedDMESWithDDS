// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");

// Hpshboss modifys code from eprosima's github example;
// Licensed under the Apache License, Version 2.0 (the "License");

/**
 * @file source.cpp
 *
 */

#include <fastrtps/Domain.h>
#include <fastrtps/log/Log.h>

#include <fstream>

using namespace eprosima;
using namespace fastrtps;
using namespace rtps;

#include <iostream>
#include <stdexcept>
#include <thread>
#include <string>
#include <cstring>
#include <vector>
#include <random>

// #include <spglog/spdlog.h>
#include <opc/common/logger.h>

#include "Machine.h"

void waitAssigedOpAndExecute(bool* portStop, bool* waitAssignedOp);
std::vector<std::string> split(const std::string& str, const std::string& delim);

std::shared_ptr<spdlog::logger> logger = spdlog::stderr_color_mt("client");
Machine machine(logger);

// Generate a order id carrier id for Intelligent
int carrierIds[10] = {1,2,3,4,5,6,7,8,9,10};
int randomCarrierId = 999;

// resourceId is given by user in command's argument
int myResourceId = 0;

int main(int argc, char ** argv){
    logger->set_level(spdlog::level::debug);
    try
    {
        myResourceId = std::atoi(argv[1]);

        bool portStop = false;
        bool portWaitAssignedOp = false;
        
        machine.monitorAssignedOperation();

        int carrierIndex = 0;
        while(true)
        {
            // 機台偵測到小車，並停止輸送帶傳送
            if (portStop && !portWaitAssignedOp)    // to publish carrier number
            {
                randomCarrierId = carrierIds[carrierIndex++];
                if (carrierIndex == 10) 
                {
                    carrierIndex = 0;
                }

                logger->debug("Stopper RFID value is {}", randomCarrierId);
                machine.broadcastCarrierPosition(myResourceId, 1, randomCarrierId, "Nope");

                portWaitAssignedOp = true;
                waitAssigedOpAndExecute(&portStop, &portWaitAssignedOp);

                /*
                std::thread waitAssigedOpAndExecuteThread(&waitAssigedOpAndExecute,
                                                          &portStop, 
                                                          &portWaitAssignedOp);
                waitAssigedOpAndExecuteThread.detach();
                */
            }
            
            // Simulate wait carrier arrival (every ten seconds)
            logger->debug("M{} waits for next carrier", myResourceId);
            for (int i = 0; i < 5; i++)
            {
                logger->debug("{} second left", 5-i);
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));  // trigger carrier arrival
            }
            portStop = true;
        }
    }

    catch (const std::exception & exc)
    {
        logger->error("Error in main: {}", exc.what());
    }

    catch (...)
    {
        logger->error("Unknown error in main.");
    }

    return -1;
}


void waitAssigedOpAndExecute(bool* portStop, 
                             bool* waitAssignedOp)
{
    logger->debug("wait assigned Operation and execute");
    for (int i = 0; i < 60; i++)  // about 3 seconds timeout
    {
        if (*machine.assignedOpSubscriber.public_messageStack)
        {
            *machine.assignedOpSubscriber.public_messageStack = false;

            int resourceId = machine.assignedOpSubscriber.public_assignedOp->resourceId();
            int portId = machine.assignedOpSubscriber.public_assignedOp->portId();
            std::string GUID = machine.assignedOpSubscriber.public_assignedOp->GUID();
            int carrierId = machine.assignedOpSubscriber.public_assignedOp->carrierId(); 
            std::string operationInfo = machine.assignedOpSubscriber.public_assignedOp->operationInfo();
            std::string note = machine.assignedOpSubscriber.public_assignedOp->note();
            
            logger->debug("assignedOperation Recieved.");

            if (resourceId == myResourceId && portId == 1 && carrierId == randomCarrierId)
            {
                if (operationInfo == "None") 
                {
                    logger->debug("No Assignment");
                    *portStop = false;
                    machine.responseAssignedOperation(resourceId, portId, GUID, carrierId, operationInfo, "NONE", "Nope");
                    *waitAssignedOp = false;
                    return; 
                }

                logger->debug("Ready to execute operation, which is {}", operationInfo);
                machine.responseAssignedOperation(resourceId, portId, GUID, carrierId, operationInfo, "READY", "Nope");

                *portStop = false;
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                *waitAssignedOp = false;

                // 模擬等待機台加工完成: 3 seconds
                logger->debug("Machine {} is on busy for three seconds", myResourceId);
                std::this_thread::sleep_for(std::chrono::milliseconds(3000));
                logger->debug("Operation Complete");

                machine.responseAssignedOperation(resourceId, portId, GUID, carrierId, operationInfo, "DONE", "NOPE");

                return;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));  // about 50 millisecond
    }

    logger->debug("No Response");
    
    *portStop = false;
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    *waitAssignedOp = false;
}


std::vector<std::string> split(const std::string& str, const std::string& delim)    // C++ has no split function
{   
	std::vector<std::string> res; 

	if("" == str) return res;  
	
	char* strs = new char[str.length() + 1];
	std::strcpy(strs, str.c_str());   

	char* d = new char[delim.length() + 1];  
	std::strcpy(d, delim.c_str());  

	char* p = strtok(strs, d); 

	while(p)
    {
		std::string s = p;
		res.push_back(s);
		p = strtok(NULL, d);  
	}  

	return res;  
}