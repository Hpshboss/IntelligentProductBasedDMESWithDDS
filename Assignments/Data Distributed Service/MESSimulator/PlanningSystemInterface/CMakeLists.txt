# // Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
# //
# // Licensed under the Apache License, Version 2.0 (the "License");

# // Hpshboss modifys code from eprosima's github example;
# // Licensed under the Apache License, Version 2.0 (the "License");

cmake_minimum_required(VERSION 2.8.12)

message("LD_LIBRARY_PATH: $ENV{LD_LIBRARY_PATH}")

if(NOT CMAKE_VERSION VERSION_LESS 3.0)
    cmake_policy(SET CMP0048 NEW)
endif()

project(PlanningSystemInterfaceExample)

# Find requirements
if(NOT fastcdr_FOUND)
    find_package(fastcdr REQUIRED)
endif()

if(NOT foonathan_memory_FOUND)
    find_package(foonathan_memory REQUIRED)
endif()

if(NOT fastrtps_FOUND)
    find_package(fastrtps REQUIRED)
endif()

find_package(spdlog)
if(spdlog_FOUND)
    message(STATUS "Found spdlog")
    set(ADDITIONAL_PUBLIC_COMPILE_OPTIONS -DHAVE_SYSTEM_SPDLOG)
    set(ADDITIONAL_LINK_LIBRARIES ${ADDITIONAL_LINK_LIBRARIES} spdlog::spdlog)
endif()

#Check C++11
include(CheckCXXCompilerFlag)
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANG OR
        CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    check_cxx_compiler_flag(-std=c++11 SUPPORTS_CXX11)
    if(NOT SUPPORTS_CXX11)
        message(FATAL_ERROR "Compiler doesn't support C++11")
    endif()
endif()

message(STATUS "Configuring PLANNINGSYSTEMINTERFACE example...")
file(GLOB PLANNINGSYSTEMINTERFACE_EXAMPLE_SOURCES_CXX "*.cxx")
file(GLOB PLANNINGSYSTEMINTERFACE_EXAMPLE_SOURCES_CPP "*.cpp")

add_executable(PlanningSystemInterfaceExample ${PLANNINGSYSTEMINTERFACE_EXAMPLE_SOURCES_CXX} ${PLANNINGSYSTEMINTERFACE_EXAMPLE_SOURCES_CPP})
target_link_libraries(PlanningSystemInterfaceExample fastrtps fastcdr foonathan_memory ${ADDITIONAL_LINK_LIBRARIES})
install(TARGETS PlanningSystemInterfaceExample
    RUNTIME DESTINATION examples/C++/PlanningSystemInterfaceExample/${BIN_INSTALL_DIR})