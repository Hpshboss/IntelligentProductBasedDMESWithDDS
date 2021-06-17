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

// #include <spglog/spdlog.h>
#include <opc/common/logger.h>

#include "MagBackMachine.h"
#include "MagBackOPCUA.h"

void waitAssigedOpAndExecute(bool* portStop, bool* waitAssignedOp, festoLab::MachineStates* machineS);
std::vector<std::string> split(const std::string& str, const std::string& delim);

std::shared_ptr<spdlog::logger> logger = spdlog::stderr_color_mt("client");
festoLab::MagBackOpcuaAgent magBackOpcuaAgent(logger);
MagBackMachine magBackMachine(logger);

int main(int argc, char ** argv){
    
    logger->set_level(spdlog::level::debug);
    try
    {
        bool portStop = false;
        bool portWaitAssignedOp = false;
        magBackOpcuaAgent.monitorCarrierArrivalThenStop(&portStop);

        festoLab::MachineStates machineState;
        magBackOpcuaAgent.monitorMachineState(&machineState);


        std::this_thread::sleep_for(std::chrono::milliseconds(1500));
        magBackMachine.monitorAssignedOperation();

        while(true)
        {
            // 機台偵測到小車，並停止輸送帶傳送
            if (portStop && !portWaitAssignedOp)
            {
                logger->debug("Stopper RFID value is {}", magBackOpcuaAgent.readRfid());
                magBackMachine.broadcastCarrierPosition(1, 1, magBackOpcuaAgent.readRfid(), "Nope");

                portWaitAssignedOp = true;
                std::thread waitAssigedOpAndExecuteThread(&waitAssigedOpAndExecute, &portStop, &portWaitAssignedOp, &machineState);
                waitAssigedOpAndExecuteThread.detach();
            }
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

void waitAssigedOpAndExecute(bool* portStop, bool* waitAssignedOp, festoLab::MachineStates* machineS)
{
    logger->debug("wait assigned Operation and execute");
    for (int i = 0; i < 60; i++)  // about 3 seconds timeout
    {
        if (*magBackMachine.assignedOpSubscriber.public_messageStack)
        {
            *magBackMachine.assignedOpSubscriber.public_messageStack = false;
            logger->debug("assignedOperation Recieved.");

            int resourceId = magBackMachine.assignedOpSubscriber.public_assignedOp->resourceId();
            int portId = magBackMachine.assignedOpSubscriber.public_assignedOp->portId();
            std::string GUID = magBackMachine.assignedOpSubscriber.public_assignedOp->GUID();
            int carrierId = magBackMachine.assignedOpSubscriber.public_assignedOp->carrierId(); 
            std::string operationInfo = magBackMachine.assignedOpSubscriber.public_assignedOp->operationInfo();
            std::string note = magBackMachine.assignedOpSubscriber.public_assignedOp->note();

            if (resourceId == 1 && portId == 1 && carrierId == magBackOpcuaAgent.readRfid())
            {
                if (operationInfo == "None") 
                {
                    logger->debug("No Assignment");
                    *portStop = false;
                    magBackMachine.responseAssignedOperation(resourceId, portId, GUID, carrierId, operationInfo, "NONE", "NOPE");
                    magBackOpcuaAgent.monitorCarrierArrivalThenStop(portStop);
                    return; 
                }

                logger->debug("Ready to execute");
                magBackMachine.responseAssignedOperation(resourceId, portId, GUID, carrierId, operationInfo, "READY", "NOPE");

                magBackOpcuaAgent.addTransition((short)1, (short)carrierId, (short)carrierId);
                magBackOpcuaAgent.transitionExecutable((short)1, true);

                while( !magBackOpcuaAgent.automatic() ) { magBackOpcuaAgent.automatic(); };
                *portStop = false;
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                *waitAssignedOp = false;

                bool onBusy = false;
                while (true)
                {
                    if (*machineS == festoLab::MachineStates::BUSY && onBusy == false)
                    {
                        logger->debug("Machine is on busy");
                        onBusy = true;
                    }
                    if (*machineS == festoLab::MachineStates::READY && onBusy == true)
                    {
                        logger->debug("Operation Complete");
                        break;
                    }
                }
                magBackOpcuaAgent.transitionExecutable((short)1, false);
                magBackMachine.responseAssignedOperation(resourceId, portId, GUID, carrierId, operationInfo, "DONE", "NOPE");

                magBackOpcuaAgent.monitorCarrierArrivalThenStop(portStop);
                return;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));  // about 50 millisecond
    }

    logger->debug("No Response");
    while( !magBackOpcuaAgent.automatic() ) { magBackOpcuaAgent.automatic(); };
    *portStop = false;
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    *waitAssignedOp = false;
    magBackOpcuaAgent.monitorCarrierArrivalThenStop(portStop);
}

std::vector<std::string> split(const std::string& str, const std::string& delim)    // C++本身沒有split
{   
	std::vector<std::string> res; 

	if("" == str) return res;  
	
	char* strs = new char[str.length() + 1] ;
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