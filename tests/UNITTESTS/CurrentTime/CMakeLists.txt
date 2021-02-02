# Copyright (c) 2020 ARM Limited. All rights reserved.
# SPDX-License-Identifier: Apache-2.0
cmake_minimum_required(VERSION 3.0.2)

set(TEST_NAME ble-service-current-time-unittest)

add_executable(${TEST_NAME})

target_include_directories(${TEST_NAME}
    PRIVATE
        .
        ${SERVICES_PATH}/CurrentTime/include
)

target_sources(${TEST_NAME}
    PRIVATE
        test_CurrentTime.cpp
        ${SERVICES_PATH}/CurrentTime/source/CurrentTimeService.cpp
)

target_link_libraries(${TEST_NAME}
    PRIVATE
        mbed-os-fakes-ble
        mbed-os-fakes-event-queue
        mbed-os-headers-base
        mbed-os-headers-platform
        mbed-os-headers-drivers
        mbed-os-headers-hal
        gmock_main
)

add_test(NAME "${TEST_NAME}" COMMAND ${TEST_NAME})
