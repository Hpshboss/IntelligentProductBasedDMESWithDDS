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
std::string GUID = "21EC2020-3AEA-1069-A2DD-08002B303099";
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

void waitAssignedOpResAndUpdateNextStepNumber(unsigned int LresourceId, unsigned int LportId, std::string LGUID, unsigned int LcarrierId, std::string LoperationInfo);
std::vector<std::string> split(const std::string& str, const std::string& delim);
int getVectorIndex(std::vector<int> vectorInstance, int elementValue);

std::shared_ptr<spdlog::logger> logger = spdlog::stderr_color_mt("client");
IntelligentProduct intelligentProduct(logger);

int main(int argc, char ** argv){
    
    logger->set_level(spdlog::level::debug); 
    try
    {
        
        logger->debug("main");

        intelligentProduct.monitorCarrierPos();
        intelligentProduct.monitorRecipeInfo();
        intelligentProduct.monitorAssignedOpRes();

        while(true)
        {
            if (onBusy == true)
            {
                if (nextStepNumber == -1)
                {
                    intelligentProduct.reportProductResult(GUID, orderNumber, orderPosition, "DONE", "Nope");
                }

                if (*intelligentProduct.carrierPosSubscriber.public_messageStack == true)
                {
                    *intelligentProduct.carrierPosSubscriber.public_messageStack = false;
                    unsigned int resourceId = intelligentProduct.carrierPosSubscriber.public_carrierPos->resourceId();
                    unsigned int portId = intelligentProduct.carrierPosSubscriber.public_carrierPos->portId();
                    unsigned int carrierId = intelligentProduct.carrierPosSubscriber.public_carrierPos->carrierId();

                    // ASRS restores and intelligent product binds a carrier
                    if (resourceIds[getVectorIndex(stepNumbers, nextStepNumber)] == resourceId && bindedCarrierId == 0) 
                    {
                        bindedCarrierId = carrierId;

                        // Operation format: "OPNo:Par_1:Par_2:...:Par_n"
                        std::string operationInfo = "";
                        int operationNumber = operationNumbers[getVectorIndex(stepNumbers, nextStepNumber)];
                        operationInfo += std::to_string(operationNumber);

                        for (int i = 0; i < detailOperationNumbers.size(); i++)
                        {
                            if (operationNumber == detailOperationNumbers[i] && resourceId == resourceIds[i])
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

                    // MagBack put, MPress press or ASRS store
                    if (resourceIds[getVectorIndex(stepNumbers, nextStepNumber)] == resourceId && bindedCarrierId == carrierId)
                    {
                        std::string operationInfo = "";
                        int operationNumber = operationNumbers[getVectorIndex(stepNumbers, nextStepNumber)];
                        operationInfo += std::to_string(operationNumber);

                        for (int i = 0; i < detailOperationNumbers.size(); i++)
                        {
                            if (operationNumber == detailOperationNumbers[i] && resourceId == resourceIds[i])
                            {
                                operationInfo += ":" + std::to_string(parameterValues[i]);
                            }
                        }

                        // resource id: MagBack=1, MPress=2, ASRS=3
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
            else
            {
                if (*intelligentProduct.recipeInfoSubscriber.public_messageStack == true)
                {
                    onBusy = true;
                    *intelligentProduct.recipeInfoSubscriber.public_messageStack = false;
                    orderNumber = intelligentProduct.recipeInfoSubscriber.public_recipeInfo->orderNumber();
                    orderPosition = intelligentProduct.recipeInfoSubscriber.public_recipeInfo->orderPosition();

                    // workInfo format: "WPNo;ResourceId_n:ONo_1:Par_1_1:...Par_1_n;...;ResurceId_n:ONo_n:Par_n_1:...Par_n_n"
                    std::vector<std::string>workedPlan = split(intelligentProduct.recipeInfoSubscriber.public_recipeInfo->workPlan(), ";");
                    for (int i = 0; workedPlan.size() - 1; i++)
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
                    nextStepNumber = stepNumbers[0];
                }
                intelligentProduct.responseRecipe(GUID, orderNumber, orderPosition, "Nope");
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

// wait assignedOpRes and update next step number
void waitAssignedOpResAndUpdateNextStepNumber(unsigned int LresourceId, 
                                              unsigned int LportId, 
                                              std::string LGUID, 
                                              unsigned int LcarrierId, 
                                              std::string LoperationInfo)
{
    while (true)  // about 2 seconds timeout
    {
        if (*intelligentProduct.assignedOpResSubscriber.public_messageStack == true)
        {
            *intelligentProduct.assignedOpResSubscriber.public_messageStack = false;
            if (LresourceId == intelligentProduct.assignedOpResSubscriber.public_assignedOpRes->resourceId() &&
                LportId == intelligentProduct.assignedOpResSubscriber.public_assignedOpRes->portId() &&
                LGUID == intelligentProduct.assignedOpResSubscriber.public_assignedOpRes->GUID() &&
                LcarrierId == intelligentProduct.assignedOpResSubscriber.public_assignedOpRes->carrierId() &&
                LoperationInfo == intelligentProduct.assignedOpResSubscriber.public_assignedOpRes->operationInfo() &&
                intelligentProduct.assignedOpResSubscriber.public_assignedOpRes->result() == "DONE")
            {
                int index = getVectorIndex(stepNumbers, nextStepNumber);
                if (index == stepNumbers.size() - 1) 
                {
                    nextStepNumber = -1;
                    break;
                }
                nextStepNumber = stepNumbers[index + 1];
                break;
            }
        }
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