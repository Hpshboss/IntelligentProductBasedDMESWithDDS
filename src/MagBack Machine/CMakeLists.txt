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

project(MagBackMachineModule)

set(ADDITIONAL_PUBLIC_COMPILE_OPTIONS)

if(MSVC)
    SET(STATIC_LIBRARY_CXX_FLAGS  /MDd CACHE STRING "")
    SET(EXECUTABLE_CXX_FLAGS /MDd  CACHE STRING "")
    SET(DYNAMIC_LIBRARY_CXX_FLAGS /MDd  CACHE STRING "")
    SET(D  /D)

    add_definitions(/D_SCL_SECURE_NO_WARNINGS /D_CRT_SECURE_NO_WARNINGS /D_WIN32 /D_WINDOWS /FS /D_WIN32_WINNT=0x0600)
    add_compile_options(/Zi /Od /EHsc /W4)
else(MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG " -Wall -ggdb -O0 ${CMAKE_CXX_FLAGS_DEBUG}")

    SET(STATIC_LIBRARY_CXX_FLAGS)
    SET(EXECUTABLE_CXX_FLAGS)
    SET(DYNAMIC_LIBRARY_CXX_FLAGS)
    SET(D -D)
    set(CMAKE_CXX_FLAGS " -std=c++11 -Wall -fPIC ${CMAKE_CXX_FLAGS} ")
    SET (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS_INIT} $ENV{LDFLAGS}")
    #set(CMAKE_SHARED_LINKER_FLAGS "--no-undefined" )
endif()

if(WIN32)
    SET(gtest_force_shared_crt "Use shared (DLL) run-time lib even when Google Test is built as static lib." ON)
    SET(ADDITIONAL_LINK_LIBRARIES Wininet.lib wsock32.lib)
    if(CMAKE_COMPILER_IS_GNUCC)
        SET(ADDITIONAL_LINK_LIBRARIES ${ADDITIONAL_LINK_LIBRARIES} ws2_32.lib)
    endif()

    SET(OS_SUFFIX _win)
    STRING(REGEX REPLACE "/" "\\\\\\\\" DYNAMIC_ADDON_PATH "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/Debug/test_dynamic_addon.dll")
    STRING(REGEX REPLACE "/" "\\\\\\\\" TEST_CORE_CONFIG_PATH "${PROJECT_SOURCE_DIR}/tests/core/configs/")

#    if(MSVC)
#        set(CMAKE_CXX_STACK_SIZE "2000000")
#    endif()

else(WIN32)
    SET(DYNAMIC_ADDON_PATH "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libtest_dynamic_addon.so")
    SET(TEST_CORE_CONFIG_PATH "${PROJECT_SOURCE_DIR}/tests/core/configs/")
    SET(OS_SUFFIX _lin)
    # gcc less then version 4.9 uses a broken std::regex implementation
    # use libxml2 to parse URL's instead
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.9.0)
        find_package(LibXml2 REQUIRED)
        message(STATUS "LibXML2 INCLUDE DIR IS: " ${LIBXML2_INCLUDE_DIR})
        include_directories( ${LIBXML2_INCLUDE_DIR} )

        SET(NO_REGEX_SUFFIX _noregex)
    endif()


    #FIXME: remove that variable and link directly when necessary!!!!
    SET(ADDITIONAL_LINK_LIBRARIES pthread dl ${LIBXML2_LIBRARIES})

endif(WIN32)


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

find_package( Boost COMPONENTS system program_options filesystem thread REQUIRED )
include_directories( ${Boost_INCLUDE_DIRS} )
link_directories( ${Boost_LIBRARY_DIRS} )
message(STATUS "Boost INCLUDE DIR IS: " ${Boost_INCLUDE_DIRS})
message(STATUS "Boost LIBRARY DIR IS: " ${Boost_LIBRARY_DIRS})

message(STATUS "SSL support using libmbedtls: " ${SSL_SUPPORT_MBEDTLS})
if (SSL_SUPPORT_MBEDTLS)
    add_definitions(-DSSL_SUPPORT_MBEDTLS)
    IF (DEFINED SSL_SUPPORT_MBEDTLS_INCLUDE_DIR)
        message(STATUS "  mbedtls library INCLUDE_DIR: " ${SSL_SUPPORT_MBEDTLS_INCLUDE_DIR})
        include_directories( ${SSL_SUPPORT_MBEDTLS_INCLUDE_DIR} )
    ENDIF ()
    IF (DEFINED SSL_SUPPORT_MBEDTLS_LIB_DIR)
        message(STATUS "  mbedtls library LIB_DIR: " ${SSL_SUPPORT_MBEDTLS_LIB_DIR})
        link_directories( ${SSL_SUPPORT_MBEDTLS_LIB_DIR} )
    ENDIF ()
    SET(SSL_SUPPORT_LINK_LIBRARIES mbedcrypto mbedx509 mbedtls)
    message(STATUS "  mbedtls LIBRARIES: " ${SSL_SUPPORT_LINK_LIBRARIES})
endif (SSL_SUPPORT_MBEDTLS)

#Check C++11
include(CheckCXXCompilerFlag)
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANG OR
        CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    check_cxx_compiler_flag(-std=c++11 SUPPORTS_CXX11)
    if(NOT SUPPORTS_CXX11)
        message(FATAL_ERROR "Compiler doesn't support C++11")
    endif()
endif()

message(STATUS "Configuring MAGBACKMACHINE module...")
file(GLOB MAGBACKMACHINE_MODULE_SOURCES_CXX "*.cxx")
file(GLOB MAGBACKMACHINE_MODULE_SOURCES_CPP "*.cpp")

include_directories(
    /usr/local/include
)

add_executable(MagBackMachineModule ${MAGBACKMACHINE_MODULE_SOURCES_CXX} ${MAGBACKMACHINE_MODULE_SOURCES_CPP})

target_compile_options(MagBackMachineModule PUBLIC ${ADDITIONAL_PUBLIC_COMPILE_OPTIONS})
target_link_libraries(MagBackMachineModule
    fastrtps
    fastcdr
    foonathan_memory
    opcuaprotocol
    opcuacore
    opcuaclient
    ${ADDITIONAL_LINK_LIBRARIES}
    ${SSL_SUPPORT_LINK_LIBRARIES}
)

install(TARGETS MagBackMachineModule
    RUNTIME DESTINATION examples/C++/MagBackMachineModule/${BIN_INSTALL_DIR})