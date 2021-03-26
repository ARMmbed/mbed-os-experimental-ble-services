# Copyright (c) 2020 ARM Limited. All rights reserved.
# SPDX-License-Identifier: Apache-2.0

add_library(ble-service-fota INTERFACE)

target_include_directories(ble-service-fota
    INTERFACE
        .
        include
)

target_sources(ble-service-fota
    INTERFACE
        source/FOTAService.cpp
)

target_link_libraries(ble-service-fota
    INTERFACE
        mbed-ble
        mbed-events
)
