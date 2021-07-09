// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");

// Hpshboss modifys code from eprosima's github example;
// Licensed under the Apache License, Version 2.0 (the "License");

/**
 * @file Subscriber.cpp
 *
 */
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/attributes/SubscriberAttributes.h>
#include <fastrtps/subscriber/Subscriber.h>
#include <fastrtps/Domain.h>

#include <thread>
#include <time.h>
#include <vector>
#include <string> 

// #include <spglog/spdlog.h>
// #include <fastrtps/log/Log.h>
#include <opc/common/logger.h>

#include "RecipeResSubscriber.h"
#include "OrderInfoSubscriber.h"
#include "RecipeInfoPublisher.h"
#include "ProductRepSubscriber.h"


using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;
using namespace eprosima::fastrtps::types;

class WorkOrderManagement
{
public:
    WorkOrderManagement(std::shared_ptr<spdlog::logger> logger) 
        : Logger(logger)
        , orderInfoSubscriber(logger)
        , recipeResSubscriber(logger)
        , recipeInfoPublisher(logger)
        , productRepSubscriber(logger)
    {
        recipeInfoPublisher.init();
    };

    ~WorkOrderManagement()
    {

    };
    
    bool assignRecipeInfo(std::string GUID, unsigned int orderNumber, unsigned int orderPosition, std::string workPlan, std::string note)
    {
        return recipeInfoPublisher.publish(GUID, orderNumber, orderPosition, workPlan, note);
    };
    
    bool monitorPlacedOrder()
    {
        return orderInfoSubscriber.init();
    };

    bool monitorRecipeRes()
    {
        return recipeResSubscriber.init();
    };

    bool monitorProductReport()
    {
        return productRepSubscriber.init();
    };

private:
    Common::Logger::SharedPtr Logger;

public:
    RecipeInfoPublisher recipeInfoPublisher;

    OrderInfoSubscriber orderInfoSubscriber;

    RecipeResSubscriber recipeResSubscriber;

    ProductRepSubscriber productRepSubscriber;
};