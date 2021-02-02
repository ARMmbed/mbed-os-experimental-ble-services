# Copyright (c) 2020 ARM Limited. All rights reserved.
# SPDX-License-Identifier: Apache-2.0
cmake_minimum_required(VERSION 3.0.2)

# add all subdirectories

file(GLOB subdirectories RELATIVE ${CMAKE_CURRENT_LIST_DIR} *)

foreach(dir ${subdirectories})
    if(IS_DIRECTORY  ${CMAKE_CURRENT_LIST_DIR}/${dir})
        add_subdirectory(${dir})
    endif()
endforeach()
