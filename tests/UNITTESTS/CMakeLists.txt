# Copyright (c) 2020 ARM Limited. All rights reserved.
# SPDX-License-Identifier: Apache-2.0
cmake_minimum_required(VERSION 3.0.2)

set(MBED_PATH ${CMAKE_CURRENT_SOURCE_DIR}/mbed-os CACHE INTERNAL "")
set(SERVICES_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../services CACHE INTERNAL "")

set(PROJECT_NAME unittests)
set(LIB_NAME MbedOS)

project(${PROJECT_NAME})

# Setup c++ standard
macro(use_cxx14)
    if (CMAKE_VERSION VERSION_LESS 3.1)
        if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++14")
        endif()
    else()
        set(CMAKE_CXX_STANDARD 14)
        set(CMAKE_CXX_STANDARD_REQUIRED ON)
    endif()
endmacro()

use_cxx14()


if (MINGW)
    # enable PRIx formatting globally
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D__STDC_FORMAT_MACROS")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__STDC_FORMAT_MACROS")
endif (MINGW)

####################
# GTEST
####################

# Download and unpack googletest at configure time
configure_file(googletest.in googletest-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download)
if (result)
    message(FATAL_ERROR "CMake failed for google test: ${result}")
endif()
execute_process(COMMAND ${CMAKE_COMMAND} --build .
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download)
if (result)
    message(FATAL_ERROR "Build failed for google test: ${result}")
endif()

# Prevent overriding the parent project's compiler/linker
# settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Add googletest directly to our build. This defines
# the gtest and gtest_main targets.
add_subdirectory(${CMAKE_BINARY_DIR}/googletest-src
        ${CMAKE_BINARY_DIR}/googletest-build
        EXCLUDE_FROM_ALL)

# The gtest/gtest_main/gmock/gmock_main targets carry header search path
# dependencies automatically when using CMake 2.8.11 or
# later.
target_include_directories(gmock_main SYSTEM BEFORE INTERFACE
        "$<BUILD_INTERFACE:${gtest_SOURCE_DIR}/include>"
        "$<BUILD_INTERFACE:${gmock_SOURCE_DIR}/include>")

####################
# TESTING
####################

include(CTest)

set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES
        "${CMAKE_BINARY_DIR}/Testing"
        )

####################
# CODE COVERAGE SETUP
####################

if (COVERAGE)

    if (NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
        message(WARNING "Non-debug build may result misleading code coverage results.")
    endif()

    # Append coverage compiler flags
    set(COVERAGE_COMPILER_FLAGS "-g -O0 --coverage")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COVERAGE_COMPILER_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COVERAGE_COMPILER_FLAGS}")

endif(COVERAGE)

if (VALGRIND)
    find_program(MEMORYCHECK_COMMAND valgrind)
endif(VALGRIND)

####################
# UNIT TESTS
####################

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DUNITTEST")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUNITTEST")

## Add mbed-os CMakeLists.txt
add_subdirectory(${MBED_PATH}/UNITTESTS/stubs)
add_subdirectory(${MBED_PATH}/UNITTESTS/fakes)

## Add testing CMakeLists.txt
add_subdirectory(Template)
add_subdirectory(LinkLoss)
