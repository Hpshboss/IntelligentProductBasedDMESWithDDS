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
#include <vector>

// #include <spglog/spdlog.h>
#include <opc/common/logger.h>

#include "IntelligentProduct.h"

// Intelligent Product State to avoid sudden order task
bool onBusy = false;

// Basic Info
std::string GUID = "21EC2020-3AEA-1069-A2DD-08002B303099";  // default
unsigned int orderNumber = 0;
unsigned int orderPosition = 0;
int nextStepNumber = 0;  // 0: intial; -1: Done
unsigned int bindedCarrierId = 0;

// worked plan
std::vector<int> workPlanNumbers;
std::vector<int> stepNumbers;
std::vector<int> operationNumbers;
std::vector<int> resourceIds;

// worked operation
std::vector<int> detailOperationNumbers;
std::vector<int> parameterNumbers;
std::vector<int> parameterValues;
std::vector<int> detailResourceIds;

void broadcastIntelliegntProductState();
void waitAssignedOpResAndUpdateNextStepNumber(unsigned int LresourceId, unsigned int LportId, std::string LGUID, unsigned int LcarrierId, std::string LoperationInfo);
std::vector<std::string> split(const std::string& str, const std::string& delim);
int getVectorIndex(std::vector<int> vectorInstance, int elementValue);
void getFakeGUID(std::string seedNum);

std::shared_ptr<spdlog::logger> logger = spdlog::stderr_color_mt("client");
IntelligentProduct intelligentProduct(logger);

int main(int argc, char ** argv){
    logger->set_level(spdlog::level::debug); 
    try
    {
        getFakeGUID(argv[1]);

        intelligentProduct.monitorCarrierPos();
        intelligentProduct.monitorRecipeInfo();
        intelligentProduct.monitorAssignedOpRes();

        std::thread broadcastIntelliegntProductStateThread(&broadcastIntelliegntProductState);
        broadcastIntelliegntProductStateThread.detach();

        while(true)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));     // Reduce CPU's usage

            // 已接收訂單，並開始執行任務
            if (onBusy == true)
            {
                // 任務完成並回報Work Order Management
                if (nextStepNumber == -1)
                {
                    logger->debug("Task Complete");
                    intelligentProduct.reportProductResult(GUID, orderNumber, orderPosition, "DONE", "Nope");
                    onBusy = false;
                    *intelligentProduct.recipeInfoSubscriber.public_messageStack = false;
                    break;
                }

                // 各機台回報停靠小車
                if (*intelligentProduct.carrierPosSubscriber.public_messageStack == true)
                {
                    
                    *intelligentProduct.carrierPosSubscriber.public_messageStack = false;
                    unsigned int resourceId = intelligentProduct.carrierPosSubscriber.public_carrierPos->resourceId();
                    unsigned int portId = intelligentProduct.carrierPosSubscriber.public_carrierPos->portId();
                    unsigned int carrierId = intelligentProduct.carrierPosSubscriber.public_carrierPos->carrierId();

                    if (carrierId == 0) 
                    {
                        logger->debug("{}th resource has error on reading rfid due to 0th carrier", resourceId);
                        continue;
                    }

                    if (carrierId == bindedCarrierId && resourceId == resourceIds[getVectorIndex(stepNumbers, nextStepNumber)])
                    {
                        logger->debug("Carrier(C) {} stops at resource(R) {} and ID of needed C and R are {} and {}", 
                                      std::to_string(carrierId), 
                                      std::to_string(resourceId), 
                                      std::to_string(bindedCarrierId),  // zero when not binded yet
                                      std::to_string(resourceIds[getVectorIndex(stepNumbers, nextStepNumber)]));
                    }

                    // First operation of a machine (ex: ASRS restores) and intelligent product binds a carrier
                    if (resourceIds[getVectorIndex(stepNumbers, nextStepNumber)] == resourceId && bindedCarrierId == 0) 
                    {
                        logger->debug("Ask ASRS to restore...");
                        bindedCarrierId = carrierId;

                        // Operation format: "OPNo:Par_1:Par_2:...:Par_n"
                        std::string operationInfo = "";
                        int operationNumber = operationNumbers[getVectorIndex(stepNumbers, nextStepNumber)];
                        operationInfo += std::to_string(operationNumber);

                        for (int i = 0; i < detailOperationNumbers.size(); i++)
                        {
                            if (operationNumber == detailOperationNumbers[i] && resourceId == detailResourceIds[i])
                            {
                                operationInfo += ":" + std::to_string(parameterValues[i]);
                            }
                        }

                        intelligentProduct.assignOperation(resourceId, portId, GUID, bindedCarrierId, operationInfo, "Nope");
                        std::thread waitAssignedOpResAndUpdateNextStepNumberThread(&waitAssignedOpResAndUpdateNextStepNumber, 
                                                                                   resourceId, 
                                                                                   portId, 
                                                                                   GUID, 
                                                                                   carrierId, 
                                                                                   operationInfo);
                        waitAssignedOpResAndUpdateNextStepNumberThread.detach();
                    }
                    // Any machine operation (ex: MagBack put, MPress press or ASRS store)
                    else if (resourceIds[getVectorIndex(stepNumbers, nextStepNumber)] == resourceId && bindedCarrierId == carrierId)
                    {
                        logger->debug("{}th resource will execute", resourceId);
                        std::string operationInfo = "";
                        int operationNumber = operationNumbers[getVectorIndex(stepNumbers, nextStepNumber)];
                        operationInfo += std::to_string(operationNumber);

                        for (int i = 0; i < detailOperationNumbers.size(); i++)
                        {
                            if (operationNumber == detailOperationNumbers[i] && resourceId == detailResourceIds[i])
                            {
                                operationInfo += ":" + std::to_string(parameterValues[i]);
                            }
                        }

                        // resource id(ex: MagBack=1, MPress=2, ASRS=3)
                        intelligentProduct.assignOperation(resourceId, portId, GUID, carrierId, operationInfo, "Nope");
                        std::thread waitAssignedOpResAndUpdateNextStepNumberThread(&waitAssignedOpResAndUpdateNextStepNumber, 
                                                                                   resourceId, 
                                                                                   portId, 
                                                                                   GUID, 
                                                                                   carrierId, 
                                                                                   operationInfo);
                        waitAssignedOpResAndUpdateNextStepNumberThread.detach();
                    }
                    // 為符合next step number操作所對應機台
                    else if (bindedCarrierId == carrierId)
                    {
                        logger->debug("{}th resource will ignore...", resourceId);
                        std::string operationInfo = "None";
                        intelligentProduct.assignOperation(resourceId, portId, GUID, carrierId, operationInfo, "Nope");
                        std::thread waitAssignedOpResAndUpdateNextStepNumberThread(&waitAssignedOpResAndUpdateNextStepNumber, 
                                                                                   resourceId, 
                                                                                   portId, 
                                                                                   GUID, 
                                                                                   carrierId, 
                                                                                   operationInfo);
                        waitAssignedOpResAndUpdateNextStepNumberThread.detach();
                    }
                }
            }
            // 無任務 No Task
            else
            {
                // 收到Work Order Management任務
                if (*intelligentProduct.recipeInfoSubscriber.public_messageStack == true)
                {
                    if (GUID == intelligentProduct.recipeInfoSubscriber.public_recipeInfo->GUID())
                    {
                        onBusy = true;
                        *intelligentProduct.recipeInfoSubscriber.public_messageStack = false;
                        orderNumber = intelligentProduct.recipeInfoSubscriber.public_recipeInfo->orderNumber();
                        orderPosition = intelligentProduct.recipeInfoSubscriber.public_recipeInfo->orderPosition();

                        logger->debug("Receive an order whose order number and position is {} and {}", orderNumber, orderPosition);

                        try 
                        {
                            // workInfo format: "WPNo;ResourceId_n:ONo_1:Par_1_1:...Par_1_n;...;ResurceId_n:ONo_n:Par_n_1:...Par_n_n"
                            std::vector<std::string>workedPlan = split(intelligentProduct.recipeInfoSubscriber.public_recipeInfo->workPlan(), ";");
                            for (int i = 0; i < workedPlan.size() - 1; i++)
                            {
                                workPlanNumbers.push_back(std::stoi(workedPlan[0]));
                                stepNumbers.push_back((i + 1) * 10);
                                std::vector<std::string> workedOperation = split(workedPlan[i + 1], ":");
                                operationNumbers.push_back(std::stoi(workedOperation[1]));
                                resourceIds.push_back(std::stoi(workedOperation[0]));
                                for (int j = 0; j < workedOperation.size() - 2; j++)
                                {
                                    detailOperationNumbers.push_back(std::stoi(workedOperation[1]));
                                    parameterNumbers.push_back(j + 1);
                                    parameterValues.push_back(std::stoi(workedOperation[j + 2]));
                                    detailResourceIds.push_back(std::stoi(workedOperation[0]));
                                }
                            }
                            logger->debug("resource id: {} {} {} {}", resourceIds[0], resourceIds[1], resourceIds[2], resourceIds[3]);
                            nextStepNumber = stepNumbers[0];
                            logger->debug("Response to WOM with ack");
                            intelligentProduct.responseRecipe(GUID, orderNumber, orderPosition, "Nope");
                        }
                        catch (const std::exception & exc)
                        {
                            onBusy = true;
                            logger->debug("Work Plan Format Is Incorrect");
                        }
                    }
                }
            }
        }
        while (true)
        {
            logger->debug("Just not terminated");
            std::this_thread::sleep_for(std::chrono::milliseconds(5000));
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

void broadcastIntelliegntProductState()
{
    while (true)
    {
        if (onBusy == false)
        {
            if (nextStepNumber == -1)
            { 
                break;
            }
            intelligentProduct.responseRecipe(GUID, 0, 0, "Nope");
            std::this_thread::sleep_for(std::chrono::milliseconds(2500));
        }
        else if (onBusy ==true)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(5000));
        }
    }
    
}


// wait assignedOpRes and update next step number
void waitAssignedOpResAndUpdateNextStepNumber(unsigned int LresourceId, 
                                              unsigned int LportId, 
                                              std::string LGUID, 
                                              unsigned int LcarrierId, 
                                              std::string LoperationInfo)
{
    for (int i = 0; i < 60; i++)  // about 3 second timeout
    {
        if (*intelligentProduct.assignedOpResSubscriber.public_messageStack == true)
        {
            *intelligentProduct.assignedOpResSubscriber.public_messageStack = false;
            // Assignment ACK, which is READY to START
            if (LresourceId == intelligentProduct.assignedOpResSubscriber.public_assignedOpRes->resourceId() &&
                LportId == intelligentProduct.assignedOpResSubscriber.public_assignedOpRes->portId() &&
                LGUID == intelligentProduct.assignedOpResSubscriber.public_assignedOpRes->GUID() &&
                LcarrierId == intelligentProduct.assignedOpResSubscriber.public_assignedOpRes->carrierId() &&
                LoperationInfo == intelligentProduct.assignedOpResSubscriber.public_assignedOpRes->operationInfo() &&
                intelligentProduct.assignedOpResSubscriber.public_assignedOpRes->result() == "READY")
            {
                logger->debug("{}th has been ready", LresourceId);
                while (true)
                {
                    if (*intelligentProduct.assignedOpResSubscriber.public_messageStack == true)
                    {
                        *intelligentProduct.assignedOpResSubscriber.public_messageStack = false;
                        // Assignment ACK, which is FINISH
                        if (LresourceId == intelligentProduct.assignedOpResSubscriber.public_assignedOpRes->resourceId() &&
                            LportId == intelligentProduct.assignedOpResSubscriber.public_assignedOpRes->portId() &&
                            LGUID == intelligentProduct.assignedOpResSubscriber.public_assignedOpRes->GUID() &&
                            LcarrierId == intelligentProduct.assignedOpResSubscriber.public_assignedOpRes->carrierId() &&
                            LoperationInfo == intelligentProduct.assignedOpResSubscriber.public_assignedOpRes->operationInfo() &&
                            intelligentProduct.assignedOpResSubscriber.public_assignedOpRes->result() == "DONE")
                        {
                            logger->debug("{}th resource's operation completed", LresourceId);
                            int index = getVectorIndex(stepNumbers, nextStepNumber);
                            if (index == stepNumbers.size() - 1) 
                            {
                                nextStepNumber = -1;
                                logger->debug("The Product is complete");
                                return;
                            }
                            nextStepNumber = stepNumbers[index + 1];
                            logger->debug("The next step number is {}", nextStepNumber);
                            return;
                        }
                    }
                }
            }

            // No Assignment ACK
            if (LresourceId == intelligentProduct.assignedOpResSubscriber.public_assignedOpRes->resourceId() &&
                LportId == intelligentProduct.assignedOpResSubscriber.public_assignedOpRes->portId() &&
                LGUID == intelligentProduct.assignedOpResSubscriber.public_assignedOpRes->GUID() &&
                LcarrierId == intelligentProduct.assignedOpResSubscriber.public_assignedOpRes->carrierId() &&
                LoperationInfo == intelligentProduct.assignedOpResSubscriber.public_assignedOpRes->operationInfo() &&
                intelligentProduct.assignedOpResSubscriber.public_assignedOpRes->result() == "NONE")
            {
                return;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    logger->debug("No Responses");
    if (LresourceId == 3 && nextStepNumber == 10)  // ASRS=3, binding carrier fails.
    {
        bindedCarrierId = 0;
    }
}

std::vector<std::string> split(const std::string& str, const std::string& delim)    // C++本身沒有split
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

int getVectorIndex(std::vector<int> vectorInstance, int elementValue)
{
    auto it = find(vectorInstance.begin(), vectorInstance.end(), elementValue);
 
    // If element was found
    if (it != vectorInstance.end())
    {
        // calculating the index
        // of K
        int index = it - vectorInstance.begin();
        return index;
    }
    else {
        // If the element is not
        // present in the vector
        return -1;
    }
}

void getFakeGUID(std::string seedNum)
{
    GUID = "";
    for (int i = 0; i < 8; i++)
    {
        GUID += seedNum[0];
    }
    GUID += "-";
    for (int i = 0; i < 4; i++)
    {
        GUID += seedNum[1];
    }
    GUID += "-";
    for (int i = 0; i < 4; i++)
    {
        GUID += seedNum[2];
    }
    GUID += "-";
    for (int i = 0; i < 4; i++)
    {
        GUID += seedNum[3];
    }
    GUID += "-";
    for (int i = 0; i < 12; i++)
    {
        GUID += seedNum[4];
    }
}