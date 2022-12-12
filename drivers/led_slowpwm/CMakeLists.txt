# SPDX-License-Identifier: Apache-2.0

if(CONFIG_LED_SLOWPWM)
  # Add hello_world_driver.h to the set of global include paths.
  zephyr_include_directories(.)

  zephyr_library()
  zephyr_library_sources(
    led_slowpwm.c
    )
endif()
