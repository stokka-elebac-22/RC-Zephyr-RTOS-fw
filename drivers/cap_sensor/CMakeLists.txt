# SPDX-License-Identifier: Apache-2.0

if(CONFIG_CAP_SENSOR)
  zephyr_include_directories(.)

  zephyr_library()
  zephyr_library_sources(
    cap_sensor.c
    )
endif()
