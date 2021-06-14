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

// #include <spglog/spdlog.h>
#include <opc/common/logger.h>

#include "ASRSMachine.h"

#include "ASRS32OPCUA.h"

void waitAssigedOpAndExecute(bool* portStop, 
                             bool* waitAssignedOp, 
                             festoLab::MachineStates* machineS);
std::vector<std::string> split(const std::string& str, const std::string& delim);
std::shared_ptr<spdlog::logger> logger = spdlog::stderr_color_mt("client");
festoLab::ASRS32OpcuaAgent asrs32OpcuaAgent(logger);
ASRSMachine asrsMachine(logger);
int main(int argc, char ** argv){
    
    logger->set_level(spdlog::level::debug);
    try
    {
        
        bool port1Stop = false;
        bool port1WaitAssignedOp = false;
        asrs32OpcuaAgent.monitorCarrierArrivalThenStopStopper1(&port1Stop);

        festoLab::MachineStates machineState;
        asrs32OpcuaAgent.monitorMachineState(&machineState);
        
        
        asrsMachine.monitorAssignedOperation();

        while(true)
        {
            if (port1Stop && !port1WaitAssignedOp)    // to publish carrier number
            {
                logger->debug("Stopper RFID value is {}", asrs32OpcuaAgent.readStopper1Rfid());
                asrsMachine.broadcastCarrierPosition(3, 1, asrs32OpcuaAgent.readStopper1Rfid(), "Nope");

                port1WaitAssignedOp = true;
                std::thread waitAssigedOpAndExecuteThread(&waitAssigedOpAndExecute,
                                                          &port1Stop, 
                                                          &port1WaitAssignedOp, 
                                                          &machineState);
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


void waitAssigedOpAndExecute(bool* portStop, 
                             bool* waitAssignedOp, 
                             festoLab::MachineStates* machineS)
{
    for (int i = 0; i < 40; i++)  // about 2 seconds timeout
    {
        if (*asrsMachine.assignedOpSubscriber.public_messageStack)
        {
            *waitAssignedOp = false;
            *asrsMachine.assignedOpSubscriber.public_messageStack = false;
            logger->debug("assignedOperation Recieved.");

            int resourceId = asrsMachine.assignedOpSubscriber.public_assignedOp->resourceId();
            int portId = asrsMachine.assignedOpSubscriber.public_assignedOp->portId();
            std::string GUID = asrsMachine.assignedOpSubscriber.public_assignedOp->GUID();
            int carrierId = asrsMachine.assignedOpSubscriber.public_assignedOp->carrierId(); 
            std::string operationInfo = asrsMachine.assignedOpSubscriber.public_assignedOp->operationInfo();
            std::string note = asrsMachine.assignedOpSubscriber.public_assignedOp->note();

            if (resourceId == 3 && portId == 1 && carrierId == asrs32OpcuaAgent.readStopper1Rfid())
            {
                if (operationInfo == "None") 
                {
                    *portStop = false;
                    asrsMachine.responseAssignedOperation(resourceId, portId, GUID, carrierId, operationInfo, "DONE", "NOPE");
                    asrs32OpcuaAgent.monitorCarrierArrivalThenStopStopper1(portStop);
                    return; 
                }

                std::vector<std::string> funAndPar = split(operationInfo, ":");
                asrs32OpcuaAgent.addTransition((short)1, (short)carrierId, std::stoi(funAndPar[0]), std::stoi(funAndPar[1]), (short)carrierId);
                asrs32OpcuaAgent.transitionExecutable((short)1, true);

                while( !asrs32OpcuaAgent.automatic() ) { asrs32OpcuaAgent.automatic(); };
                *portStop = false;

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
                asrs32OpcuaAgent.transitionExecutable((short)1, false);
                asrsMachine.responseAssignedOperation(resourceId, portId, GUID, carrierId, operationInfo, "DONE", "NOPE");

                asrs32OpcuaAgent.monitorCarrierArrivalThenStopStopper1(portStop);
                return;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));  // about 50 millisecond
    }

    logger->debug("No Assignment");
    *portStop = false;
    asrs32OpcuaAgent.monitorCarrierArrivalThenStopStopper1(portStop);
}


std::vector<std::string> split(const std::string& str, const std::string& delim)    // C++?祈澈瘝?split
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