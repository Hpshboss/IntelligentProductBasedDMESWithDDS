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

#include "MPressMachine.h"
#include "MPressOPCUA.h"

void waitAssigedOpAndExecute(bool* portStop, bool* waitAssignedOp, festoLab::MachineStates* machineS);
std::vector<std::string> split(const std::string& str, const std::string& delim);

std::shared_ptr<spdlog::logger> logger = spdlog::stderr_color_mt("client");
festoLab::MPressOpcuaAgent mPressOpcuaAgent(logger);
MPressMachine mPressMachine(logger);

int main(int argc, char ** argv){
    logger->set_level(spdlog::level::debug);
    try
    {
        bool portStop = false;
        bool portWaitAssignedOp = false;
        mPressOpcuaAgent.monitorCarrierArrivalThenStop(&portStop);

        festoLab::MachineStates machineState;
        mPressOpcuaAgent.monitorMachineState(&machineState);

        std::this_thread::sleep_for(std::chrono::milliseconds(1500));
        mPressMachine.monitorAssignedOperation();

        // std::this_thread::sleep_for(std::chrono::milliseconds(1500));
        // mPressMachine.broadcastCarrierPosition(3, 1, 5, "Nope");
        // std::this_thread::sleep_for(std::chrono::milliseconds(1500));
        // mPressMachine.broadcastCarrierPosition(3, 1, 6, "Nope");
        // std::this_thread::sleep_for(std::chrono::milliseconds(1500));
        // mPressMachine.broadcastCarrierPosition(3, 1, 7, "Nope");

        // std::this_thread::sleep_for(std::chrono::milliseconds(1500));
        // mPressMachine.responseAssignedOperation(3, 1, "21EC2020-3AEA-1069-A2DD-08002B303099", 5, "210:10:10;", "DONE", "Nope");
        // std::this_thread::sleep_for(std::chrono::milliseconds(1500));
        // mPressMachine.responseAssignedOperation(3, 1, "21EC2020-3AEA-1069-A2DD-08002B303099", 5, "310:10:10;", "DONE", "Nope");
        // std::this_thread::sleep_for(std::chrono::milliseconds(1500));
        // mPressMachine.responseAssignedOperation(3, 1, "21EC2020-3AEA-1069-A2DD-08002B303099", 5, "410:10:10;", "DONE", "Nope");

        while(true)
        {
            if (portStop && !portWaitAssignedOp)
            {
                logger->debug("Stopper RFID value is {}", mPressOpcuaAgent.readRfid());
                mPressMachine.broadcastCarrierPosition(3, 1, mPressOpcuaAgent.readRfid(), "Nope");

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
    for (int i = 0; i < 40; i++)  // about 2 seconds timeout
    {
        if (*mPressMachine.assignedOpSubscriber.public_messageStack)
        {
            *waitAssignedOp = false;
            *mPressMachine.assignedOpSubscriber.public_messageStack = false;
            logger->debug("assignedOperation Recieved.");

            int resourceId = mPressMachine.assignedOpSubscriber.public_assignedOp->resourceId();
            int portId = mPressMachine.assignedOpSubscriber.public_assignedOp->portId();
            std::string GUID = mPressMachine.assignedOpSubscriber.public_assignedOp->GUID();
            int carrierId = mPressMachine.assignedOpSubscriber.public_assignedOp->carrierId(); 
            std::string operationInfo = mPressMachine.assignedOpSubscriber.public_assignedOp->operationInfo();
            std::string note = mPressMachine.assignedOpSubscriber.public_assignedOp->note();

            if (resourceId == 3 && portId == 2 && carrierId == mPressOpcuaAgent.readRfid())
            {
                if (operationInfo == "None") 
                {
                    *portStop = false;
                    mPressMachine.responseAssignedOperation(resourceId, portId, GUID, carrierId, operationInfo, "DONE", "NOPE");
                    mPressOpcuaAgent.monitorCarrierArrivalThenStop(portStop);
                    return; 
                }

                std::vector<std::string> funAndPar = split(operationInfo, ":");  // function and parameter
                mPressOpcuaAgent.addTransition((short)1, (short)carrierId, std::stoi(funAndPar[0]), std::stoi(funAndPar[1]), (short)carrierId);
                mPressOpcuaAgent.transitionExecutable((short)1, true);

                while( !mPressOpcuaAgent.automatic() ) { mPressOpcuaAgent.automatic(); };
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
                mPressOpcuaAgent.transitionExecutable((short)1, false);
                mPressMachine.responseAssignedOperation(resourceId, portId, GUID, carrierId, operationInfo, "DONE", "NOPE");

                mPressOpcuaAgent.monitorCarrierArrivalThenStop(portStop);
                return;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));  // about 50 millisecond
    }

    *portStop = false;
    mPressOpcuaAgent.monitorCarrierArrivalThenStop(portStop);
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