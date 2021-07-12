# Copyright (c) 2020 ARM Limited. All rights reserved.
# SPDX-License-Identifier: Apache-2.0
cmake_minimum_required(VERSION 3.0.2)

set(SERVICES_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../services CACHE INTERNAL "")
set(mbed-os_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/mbed-os CACHE INTERNAL "")

project(unittests)

include(CTest)
add_definitions(-DUNITTEST)
add_subdirectory(mbed-os/platform/tests/UNITTESTS/doubles)
add_subdirectory(mbed-os/drivers/tests/UNITTESTS/doubles)
add_subdirectory(mbed-os/events/tests/UNITTESTS/doubles)
add_subdirectory(mbed-os/rtos/tests/UNITTESTS/doubles)
add_subdirectory(mbed-os/hal/tests/UNITTESTS/doubles)
add_subdirectory(mbed-os/UNITTESTS)

add_subdirectory(Template)
add_subdirectory(LinkLoss)
add_subdirectory(DeviceInformation)
