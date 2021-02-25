# Copyright (c) 2020 ARM Limited. All rights reserved.
# SPDX-License-Identifier: Apache-2.0

add_library(ble-service-disconnection INTERFACE)

target_include_directories(ble-service-disconnection
        INTERFACE
        .
        include
        )

target_sources(ble-service-disconnection
        INTERFACE
        source/DisconnectionService.cpp
        )

target_link_libraries(ble-service-disconnection
        INTERFACE
        mbed-ble
        )
