list(APPEND ZEPHYR_EXTRA_MODULES
  ${CMAKE_CURRENT_SOURCE_DIR}/drivers/as7341
  ${CMAKE_CURRENT_SOURCE_DIR}/drivers/pca9632
  ${CMAKE_CURRENT_SOURCE_DIR}/drivers/led_slowpwm
  ${CMAKE_CURRENT_SOURCE_DIR}/drivers/cap_sensor
  )
set(BOARD_ROOT ${CMAKE_CURRENT_LIST_DIR})
set(BOARD particle_xenon)
# ===== DO NOT REMOVE =====
cmake_minimum_required(VERSION 3.13.1)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
# ===== DO NOT REMOVE =====
project(horticulture_led_fw)

FILE(GLOB app_sources src/*.c)

target_sources(app PRIVATE ${app_sources})

# Define the two required variables before including
# the source code for watching a git repository.
set(PRE_CONFIGURE_FILE "src/headers/git.h.in")
set(POST_CONFIGURE_FILE "src/headers/git.h")
include(git_watcher.cmake)

# This last instruction is critical because it ensures
# we check git for changes prior to building the demo target.
add_dependencies(app check_git)

