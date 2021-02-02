# Copyright (c) 2020 ARM Limited. All rights reserved.
# SPDX-License-Identifier: Apache-2.0
cmake_minimum_required(VERSION 3.0.2)

set(TEST_NAME ble-service-current-time-unittest)

add_executable(${TEST_NAME})

target_sources(${TEST_NAME}
    PRIVATE
        test_CurrentTime.cpp
)

target_link_libraries(${TEST_NAME}
    PRIVATE
        mbed-os-stubs-headers
        mbed-os-ble-fake
        gmock_main
)

add_test(NAME "${TEST_NAME}" COMMAND ${TEST_NAME})
