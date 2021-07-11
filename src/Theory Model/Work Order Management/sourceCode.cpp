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
            std::this_thread::sleep_for(std::chrono::milliseconds(10));     // Reduce CPU's usage

            // 收到來自Planning System Interface的訂單
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
                    orderPositions.push_back(i + 1);
                    GUIDs.push_back("");
                    positionStates.push_back("WAIT");
                }
            }

            // Intelligent Product 在生產完成後會回報的訊息
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
                            logger->debug("GUID, {}, has done for ONo, {}, and OPos, {}.", GUID, orderNumber, orderPosition);
                            positionStates[index] = "DONE";
                        }
                    }
                }
            }
            
            // Intelligent Product 廣播自身狀況或回報接收訂單成功
            if (*workOrderManagement.recipeResSubscriber.public_messageStack == true)
            {
                *workOrderManagement.recipeResSubscriber.public_messageStack = false;

                std::string GUID = workOrderManagement.recipeResSubscriber.public_recipeRes->GUID();
                unsigned int orderNumber = workOrderManagement.recipeResSubscriber.public_recipeRes->orderNumber();
                unsigned int orderPosition = workOrderManagement.recipeResSubscriber.public_recipeRes->orderPosition();

                // 當Intelligent Product為廣播狀態，且為沒任務狀態
                if (orderNumber == 0 && orderPosition == 0)
                {
                    int index = getVectorIndex(positionStates, "WAIT");
                    
                    int workPlanNumber = 0;

                    // Festo實際產品編號
                    if (index > -1 && detailPartNumbers[index] == 1215)
                    {
                        logger->debug("Delegate No Drilling and No Heating Product");
                        workPlanNumber = 1215;
                        // 委派任務
                        workOrderManagement.assignRecipeInfo(GUID, 
                                                             orderNumbers[index], 
                                                             orderPositions[index], 
                                                             getWorkPlanPackage(workPlanNumber), 
                                                             "Nope");
                    }

                    // 因模組化需求設計的虛擬產品編號
                    if (index > -1 && detailPartNumbers[index] == 6666)
                    {
                        logger->debug("Delegate Virtual Product Simulated in Festo");
                        workPlanNumber = 6666;
                        // 委派任務
                        workOrderManagement.assignRecipeInfo(GUID, 
                                                             detailOrderNumbers[index], 
                                                             orderPositions[index], 
                                                             getWorkPlanPackage(workPlanNumber), 
                                                             "Nope");
                    }
                }
                
                // 收到Intelligent Product接收訂單成功
                for (int i = 0; i < detailOrderNumbers.size(); i++)
                {
                    if (detailOrderNumbers[i] == orderNumber &&
                        orderPositions[i] == orderPosition)
                    {
                        // 更新資料庫
                        GUIDs[i] = GUID;
                        positionStates[i] = "ONLINE";
                    }
                }
                
                // 防止同一個Intelligent Product在Work Order Management處理訂單在廣播一次狀態
                if (*workOrderManagement.recipeResSubscriber.public_messageStack == true)
                {
                    *workOrderManagement.recipeResSubscriber.public_messageStack = false;
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
    workPlanNumbers = {
        1215, 1215, 1215, 1215,     // refer to FestoMes.accdb
        6666, 6666, 6666, 6666, 6666
    };  

    stepNumbers = {
        10, 20, 30, 40,
        10, 20, 30, 40, 50
    };

    // 212: release a defined part on stopper 1
    // 201: feed back cover from magazine
    // 111: pressing with force regulation
    // 210: store a part from stopper 1
    operationNumbers = {
        // work plan 1215
        212, 201, 111, 210,

        // work plan 6666
        212, 400, 500, 600, 210
    };

    resourceIds = {
        // work plan 1215
        3, 1, 2, 3,     // 1: MagBack; 2: MPress; 3: ASRS

        // work plan 6666
        3, 4, 5, 6, 3   // 4, 5, 6 are resource id(s) of virtual resources
    }; 

    //**************************Operations*************************//
    detailOperationNumbers = {
        // work plan 1215
        212, 212, 111, 111, 210, 210,

        // work plan 6666
        212, 212, 210, 210
    };

    parameterNumbers = {
        // work plan 1215
        1, 2, 1, 2, 1, 2,

        // work plan 6666
        1, 2, 1, 2
    };

    parameterValues = {
        // work plan 1215
        12, 210,  // 12: restore on stopper 1; 210: certain part number
        40, 10,   // 40: press[N], 10: time[s]
        11, 1215,    // 11: store on stopper 1

        // work plan 6666
        12, 210,
        11, 6666
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