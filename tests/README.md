# Testing experimental services

The services provided are covered by unittests. We use GoogleTest as the testing framework and CTest
as the runner.

When adding a new service please include a unittest suite for your new service in your service PR.
You are free to use any framework as long as it integrates with our CTest runner but to take advantage of
mocks provided by mbed-os you'll have to use [GoogleTest](https://github.com/google/googletest).

# Running

To run unittests simply run:

```
./unittests.sh
```

This will configure and build the tests with Cmake and run the tests.

Unit tests require mbed-os to provide stubs and mocks. A copy of mbed-os will be checked out during the build into
the tests directory unless it's already there (you may use a symlink if you already have a checkout of mbed-os):

```
tests/
└── UNITESTS/
└── mbed-os/
```

This will not be deleted after the run. If you already have a symlinked copy of mbed-os in this place it will not
be affected and the unit tests will use it as is.

# Adding a new test

Before or during the development of your service you should add a new unittest to cover it. This is done by
creating a new directory inside [UNITTESTS](./UNITTESTS). CMake will automatically add any directory added there.

Inside the directory you must add a `CMakeLists.txt` file. You may use this template below. Please change the
"your service" values to your actual service name.

```
# Copyright (c) 2020 ARM Limited. All rights reserved.
# SPDX-License-Identifier: Apache-2.0
cmake_minimum_required(VERSION 3.0.2)

# change your-service to the name of your service
set(TEST_NAME ble-service-your-service-unittest)

add_executable(${TEST_NAME})

target_include_directories(${TEST_NAME}
    PRIVATE
        .
        # change the path to where your includes are
        ${SERVICES_PATH}/YourService/include
)

target_sources(${TEST_NAME}
    PRIVATE
        # replace with the source file you have your unit test in
        test_YourService.cpp
        # replace the source file with the one that compiles your service
        ${SERVICES_PATH}/YourService/source/YourService.cpp
)

target_link_libraries(${TEST_NAME}
    PRIVATE
        # add any stubs, headers or mocks you need here
        # you will most likely want to include:
        # mbed-os-fakes-ble
        gmock_main
)

add_test(NAME "${TEST_NAME}" COMMAND ${TEST_NAME})
```

Inside CMake files for unit tests there are two special variables you can use that are set by the top level
unit test CMakeLists.txt: `MBED_PATH`, `SERVICES_PATH`. These point to mbed-os and the services directory.

## Stubs, mocks and fakes in mbed-os

To make it easier to test your service mbed-os provides some ready made CMake libraries you can add in your
`target_link_libraries` section (see template above). Libraries can be object files that contains stubs, mocks
and fakes but also sets of include paths - this way you can gain access to normal headers from mbed-os.

All the libraries available are in the `CMakeLists.txt` files in `mbed-os/UNITTESTS/stubs` and
`mbed-os/UNITTESTS/fakes`.

The most common ones you might need are `mbed-os-fakes-ble` and `mbed-os-fakes-event-queue`.

### mbed-os-fakes-ble

This library provides a fake BLE implementation that uses mocks instead of real BLE components for `Gap`,
`GattServer`, `GattClient`, `SecurityManager`.

There is no need to initialise a fake BLE instance; it is ready to go and can be used just like a normal BLE instance:

```
BLE ble = &BLE::Instance();
```

This call also initialises mocks. Do no cache the BLE instance pointer, or pointer to GAP, GattServer etc. between
tests. You must get the instance fresh at the start of the test.

You can retrieve all the BLE APIs from the instance just like with a normal one:

```
Gap &gap = ble->gap();
GattClient &client = ble->gattClient();
GattServer &server = ble->gattServer();
SecurityManager &sm = ble->securityManager();
```

Whenever an API call is made, the implementation will be called. These are replaced in the fake BLE with google mocks.
This means you can set expectations on them. 

```
EXPECT_CALL(ble::gap_mock(), reset());
```

This will set up an expectations that at some point during the test the Gap::reset method will be called.

The way google test works means that if you set any expectations on your mocks they must be destroyed at
the end of each test. This is done through the fake BLE instance special method:

```
ble::delete_mocks();
```

### mbed-os-fakes-event-queue

This is a fake event queue that doesn't bring in any dependencies from mbed-os. Its API is simplified
and it only offers limited functionality.

If you choose to use it you must not also include a library that brings in real headers for the event
queue as they would conflict.

The API calls supported are for simple calls `call`, `call_in` and the `cancel` method.

The event queue is not run in real time and must be progressed manually. You may use
`dispatch(int milliseconds)` and `dispatch_forever()` to process events in the queue. This way you can
simulate the passage of time in your test. 
