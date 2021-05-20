# Copyright (c) 2020 ARM Limited. All rights reserved.
# SPDX-License-Identifier: Apache-2.0

add_library(ble-service-device-information INTERFACE)

target_include_directories(ble-service-device-information
    INTERFACE
        .
        include
)

target_sources(ble-service-device-information
    INTERFACE
        source/DeviceInformationService.cpp
)

target_link_libraries(ble-service-device-information
    INTERFACE
        mbed-ble
)
