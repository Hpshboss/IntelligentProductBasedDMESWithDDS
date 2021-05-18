// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");

// Hpshboss modifys code from eprosima's github example;
// Licensed under the Apache License, Version 2.0 (the "License");

/**
 * @file Publisher.cpp
 *
 */
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/attributes/PublisherAttributes.h>
#include <fastrtps/publisher/Publisher.h>
#include <fastrtps/Domain.h>

#include <thread>
#include <time.h>
#include <vector>
#include <string> 

// #include <spglog/spdlog.h>
// #include <fastrtps/log/Log.h>
#include <opc/common/logger.h>

#include "RecipeResPublisher.h"
#include "AssignedOpPublisher.h"
#include "CarrierPosSubscriber.h"
#include "RecipeInfoSubscriber.h"
#include "ProductRepPublisher.h"
#include "AssignedOpResSubscriber.h"

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;
using namespace eprosima::fastrtps::types;

class IntelligentProduct
{
public:
    IntelligentProduct(std::shared_ptr<spdlog::logger> logger) 
        : Logger(logger)
        , recipeResPublisher(logger)
        , assignedOpPublisher(logger)
        , carrierPosSubscriber(logger)
        , recipeInfoSubscriber(logger)
        , productRepPublisher(logger)
        , assignedOpResSubscriber(logger)
    {
        recipeResPublisher.init();
        assignedOpPublisher.init();
        productRepPublisher.init();
    };

    ~IntelligentProduct()
    {

    };

    bool responseRecipe(std::string GUID, unsigned int orderNumber, unsigned int orderPosition, std::string note)
    {
        Logger->debug("response recipe");
        recipeResPublisher.publish(GUID, orderNumber, orderPosition, note);
    };

    bool assignOperation(unsigned int resourceId, unsigned int portId, unsigned int orderNumber, unsigned int orderPosition, std::string operationInfo, std::string note)
    {
        Logger->debug("assign operation");
        assignedOpPublisher.publish(resourceId, portId, orderNumber, orderPosition, operationInfo, note);
    };

    bool monitorCarrierPos()
    {
        carrierPosSubscriber.init();
    };

    bool monitorRecipeInfo()
    {
        recipeInfoSubscriber.init();
    };

    bool reportProductResult(std::string GUID, unsigned int orderNumber, unsigned int orderPosition, std::string result, std::string note)
    {
        productRepPublisher.publish(GUID, orderNumber, orderPosition, result, note);
    };

    bool monitorAssignedOpRes()
    {
        assignedOpResSubscriber.init();
    };

private:
    Common::Logger::SharedPtr Logger;
    RecipeResPublisher recipeResPublisher;
    AssignedOpPublisher assignedOpPublisher;
    CarrierPosSubscriber carrierPosSubscriber;
    RecipeInfoSubscriber recipeInfoSubscriber;
    ProductRepPublisher productRepPublisher;
    AssignedOpResSubscriber assignedOpResSubscriber;
};