# Copyright (c) 2020 ARM Limited. All rights reserved.
# SPDX-License-Identifier: Apache-2.0
cmake_minimum_required(VERSION 3.0.2)

set(MBED_PATH ${CMAKE_CURRENT_SOURCE_DIR}/mbed-os CACHE INTERNAL "")
set(SERVICES_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../services CACHE INTERNAL "")

project(unittests)

enable_testing()

add_subdirectory(${MBED_PATH}/UNITTESTS)

add_subdirectory(UNITTESTS)


