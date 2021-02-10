# Copyright (c) 2020 ARM Limited. All rights reserved.
# SPDX-License-Identifier: Apache-2.0
cmake_minimum_required(VERSION 3.0.2)

set(TEST_NAME ble-service-link-loss-unittest)

add_executable(${TEST_NAME})

target_include_directories(${TEST_NAME}
    PRIVATE
        .
        ${SERVICES_PATH}/LinkLoss/include
        ${MBED_PATH}/connectivity/FEATURE_BLE/include/ble/gap
)

target_sources(${TEST_NAME}
    PRIVATE
        test_LinkLossService.cpp
        ${SERVICES_PATH}/LinkLoss/source/LinkLossService.cpp
)

target_link_libraries(${TEST_NAME}
    PRIVATE
        mbed-os-fakes-ble
        mbed-os-fakes-event-queue
        mbed-os-headers-base
        mbed-os-headers-platform
        gmock_main
)

add_test(NAME "${TEST_NAME}" COMMAND ${TEST_NAME})
