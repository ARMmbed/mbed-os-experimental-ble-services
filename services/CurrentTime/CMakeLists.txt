# Copyright (c) 2020 ARM Limited. All rights reserved.
# SPDX-License-Identifier: Apache-2.0

add_library(ble-service-current-time INTERFACE)

target_include_directories(ble-service-current-time
    INTERFACE
        . 
        include
)

target_sources(ble-service-current-time
    INTERFACE
        source/CurrentTimeService.cpp
)

target_link_libraries(ble-service-current-time
    INTERFACE
        mbed-ble
        mbed-events
        mbed-core
)


