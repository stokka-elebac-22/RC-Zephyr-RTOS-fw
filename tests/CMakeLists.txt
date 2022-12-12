# SPDX-License-Identifier: Apache-2.0

list(APPEND ZEPHYR_EXTRA_MODULES
  ${CMAKE_CURRENT_SOURCE_DIR}/../drivers/as7341
  ${CMAKE_CURRENT_SOURCE_DIR}/../drivers/pca9632
  ${CMAKE_CURRENT_SOURCE_DIR}/../drivers/led_slowpwm
  ${CMAKE_CURRENT_SOURCE_DIR}/../drivers/cap_sensor
  )
set(BOARD_ROOT ${CMAKE_CURRENT_LIST_DIR}/..)
set(BOARD particle_xenon)
cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(horti_fw)
FILE(GLOB app_sources ../src/*.c)
FILE(GLOB test_sources src/*.c)
list(REMOVE_ITEM app_sources  "${CMAKE_CURRENT_SOURCE_DIR}/../src/main.c")
message(STATUS "app_sources: ${app_sources}")
message(STATUS "file: ${CMAKE_CURRENT_SOURCE_DIR}/../src/main.c")
target_sources(app PRIVATE ${test_sources} ${app_sources})
zephyr_include_directories(../src)
list(REMOVE_ITEM zephyr_cc_option "-fno-pic")
list(REMOVE_ITEM zephyr_cc_option "-fno-pie")

# Define the two required variables before including
# the source code for watching a git repository.
set(PRE_CONFIGURE_FILE "../src/headers/git.h.in")
set(POST_CONFIGURE_FILE "../src/headers/git.h")
include(../git_watcher.cmake)

# This last instruction is critical because it ensures
# we check git for changes prior to building the demo target.
add_dependencies(app check_git)

