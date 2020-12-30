# Copyright (c) 2020 ARM Limited. All rights reserved.
# SPDX-License-Identifier: Apache-2.0

add_library(ble-service-link-loss INTERFACE)

target_include_directories(ble-service-link-loss
    INTERFACE
        .
        include
)

target_sources(ble-service-link-loss
    INTERFACE
        source/LinkLossService.cpp
)

target_link_libraries(ble-service-link-loss
    INTERFACE
        mbed-ble
        mbed-events
)
