# Copyright (c) 2020 ARM Limited. All rights reserved.
# SPDX-License-Identifier: Apache-2.0
cmake_minimum_required(VERSION 3.0.2)

set(SERVICES_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../services CACHE INTERNAL "")
set(mbed-os_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/mbed-os CACHE INTERNAL "")

project(unittests)

include(CTest)
add_subdirectory(mbed-os/UNITTESTS)

add_subdirectory(Template)
add_subdirectory(LinkLoss)
add_subdirectory(DeviceInformation)
