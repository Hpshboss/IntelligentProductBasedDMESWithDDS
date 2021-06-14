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

#include "WorkOrderManagement.h"


// Orders
std::vector<unsigned int> orderNumbers;
std::vector<unsigned int> partNumbers;  // product number
std::vector<unsigned short> quantitys;
std::vector<std::string> orderStates;

// Order Details
std::vector<unsigned int> detailOrderNumbers;
std::vector<unsigned int> detailPartNumbers;
std::vector<unsigned int> orderPositions;
std::vector<std::string> GUIDs;
std::vector<std::string> positionStates;

// Work Plans
std::vector<int> workPlanNumbers;
std::vector<int> stepNumbers;
std::vector<int> operationNumbers;
std::vector<unsigned int> resourceIds;

// Operations
std::vector<int> detailOperationNumbers;
std::vector<int> parameterNumbers;
std::vector<int> parameterValues;

void initializeDatabase();
std::string getWorkPlanPackage(int workPlanNumber);
int getVectorIndex(std::vector<int> vectorInstance, int elementValue);
int getVectorIndex(std::vector<std::string> vectorInstance, std::string elementValue);


std::shared_ptr<spdlog::logger> logger = spdlog::stderr_color_mt("client");
WorkOrderManagement workOrderManagement(logger);


int main(int argc, char ** argv){
    
    logger->set_level(spdlog::level::debug); 
    try
    {
        initializeDatabase();
        
        workOrderManagement.monitorPlacedOrder();
        workOrderManagement.monitorRecipeRes();
        workOrderManagement.monitorProductReport();

        logger->debug("Wait 5 seconds");

        while(true)
        {
            if (*workOrderManagement.orderInfoSubscriber.public_messageStack == true)
            {
                *workOrderManagement.orderInfoSubscriber.public_messageStack = false;

                unsigned int orderNumber = workOrderManagement.orderInfoSubscriber.public_orderInfo->orderNumber();
                unsigned int partNumber = workOrderManagement.orderInfoSubscriber.public_orderInfo->partNumber();
                unsigned int quantity = workOrderManagement.orderInfoSubscriber.public_orderInfo->quantity();

                orderNumbers.push_back(orderNumber);
                partNumbers.push_back(partNumber);
                quantitys.push_back(quantity);
                orderStates.push_back("WAIT");

                for (int i = 0; i < quantity; i++)
                {
                    detailOrderNumbers.push_back(orderNumber);
                    detailPartNumbers.push_back(partNumber);
                    orderPositions.push_back(quantity);
                    GUIDs.push_back("");
                    positionStates.push_back("WAIT");
                }
            }

            if (*workOrderManagement.productRepSubscriber.public_messageStack == true)
            {
                *workOrderManagement.productRepSubscriber.public_messageStack = false;

                std::string GUID = workOrderManagement.productRepSubscriber.public_productRep->GUID();
                unsigned int orderNumber = workOrderManagement.productRepSubscriber.public_productRep->orderNumber();
                unsigned int orderPosition = workOrderManagement.productRepSubscriber.public_productRep->orderPosition();
                std::string result = workOrderManagement.productRepSubscriber.public_productRep->result();
                
                int index = getVectorIndex(GUIDs, GUID);
                if (index > -1)
                {
                    if (detailOrderNumbers[index] == orderNumber &&
                        orderPositions[index] == orderPosition &&
                        GUIDs[index] == GUID)
                    {
                        if (result == "DONE")
                        {
                            positionStates[index] = "DONE";
                        }
                    }
                }
            }

            if (*workOrderManagement.recipeResSubscriber.public_messageStack == true)
            {
                *workOrderManagement.recipeResSubscriber.public_messageStack = false;

                std::string GUID = workOrderManagement.productRepSubscriber.public_productRep->GUID();
                unsigned int orderNumber = workOrderManagement.productRepSubscriber.public_productRep->orderNumber();
                unsigned int orderPosition = workOrderManagement.productRepSubscriber.public_productRep->orderPosition();

                if (orderNumber == 0 && orderPosition == 0)
                {
                    int index = getVectorIndex(positionStates, "WAIT");
                    int workPlanNumber = 0;
                    if (detailPartNumbers[index] == 1214)
                    {
                        workPlanNumber = 1214;
                    }
                    workOrderManagement.assignRecipeInfo(GUID, 
                                                         orderNumbers[index], 
                                                         orderPositions[index], 
                                                         getWorkPlanPackage(workPlanNumber), 
                                                         "Nope");
                }

                for (int i = 0; i < orderNumbers.size(); i++)
                {
                    if (orderNumbers[i] == orderNumber &&
                        orderPositions[i] == orderPosition)
                    {
                        GUIDs[i] = GUID;
                    }
                }
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

void initializeDatabase()
{
    //*************************Work Plans*************************//
    workPlanNumbers = {1214, 1214, 1214, 1214};  // refer to FestoMes.accdb
    stepNumbers = {10, 20, 30, 40};

    // 212: release a defined part on stopper 1
    // 201: feed back cover from magazine
    // 111: pressing with force regulation
    // 210: store a part from stopper 1
    operationNumbers = {212, 201, 111, 210};
    resourceIds = {3, 1, 2, 3};  // 1: MagBack; 2: MPress; 3: ASRS

    //**************************Operations*************************//
    detailOperationNumbers = {212, 212, 111, 111, 210, 210};
    parameterNumbers = {1, 2, 1, 2, 1, 2};
    parameterValues = {
        12, 210,  // 12: restore on stopper 1; 210: certain part number
        40, 10,   // 40: press[N], 10: time[s]
        11, 0    // 11: store on stopper 1
    };
}

std::string getWorkPlanPackage(int workPlanNumber)
{
    std::string package = "";
    if (getVectorIndex(workPlanNumbers, workPlanNumber) > -1)
    {
        package += std::to_string(workPlanNumber);
    }
    for (int i = 0; i < workPlanNumbers.size(); i++)
    {
        if (workPlanNumbers[i] == workPlanNumber)
        {
            package += ";" + std::to_string(resourceIds[i]);
            package += ":" + std::to_string(operationNumbers[i]);
            for (int j = 0; j < detailOperationNumbers.size(); j++)
            {
                if (detailOperationNumbers[j] == operationNumbers[i])
                {
                    package += ":" + std::to_string(parameterValues[j]);
                }
            }
        }
    }
    return package;
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

int getVectorIndex(std::vector<std::string> vectorInstance, std::string elementValue)
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