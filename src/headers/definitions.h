/*
 * Copyright (c) 2021
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef SRC_HEADERS_DEFINITIONS_H_
#define SRC_HEADERS_DEFINITIONS_H_
#define MAX_SENSOR_CHANNELS 8
#define MAX_ACTIVE_SENSORS 5
#define MAX_SYSTEM_TIMERS 5
#define SETTINGS_MAJ_VER 1
#define SETTINGS_MIN_VER 0
#define MAX_DISTANCE_SENSORS 2
#define MAX_SENSOR_READINGS 20

#define WQ_TIME_S 1

#include <drivers/sensor.h>

struct sensordata {
    uint8_t measurement_type;
    uint8_t retries;  // If no data in X attempts, skip from now on (?)
    struct sensor_value data_channel;
};

struct settings_version {
    uint8_t major;
    uint8_t minor;
};

struct log_timer {
    uint8_t sensor_id;
    uint32_t timer_interval;
};

struct config_struct {
    struct settings_version setting_version;    // Version of the settings struct
    uint32_t setting_timestamp;
    uint32_t system_timers[MAX_SYSTEM_TIMERS];
};

enum sensor_ids {
    BME280_0,
    MCP6500_0,
    VL53L0X_0,
};

enum motor_ids {
    MOTOR_LEFT,
    MOTOR_RIGHT,
    MOTOR_EXT,
    MOTOR_SERVO,
    DIR_FORWARD,
    DIR_REVERSE,
};

enum LOG_DATA {
    BME280_AMBIENT_TEMP,
    BME280_PRESS,
    BME280_HUMIDITY,
    VL53L0X_DISTANCE,
    VL53L0X_PROX,
    HCSR04_0,
    HCSR04_1,
};

enum status_code {
    UNKNOWN,
    ERROR,
    RUNNING_OK,
    DOWNLOADING,
    FLASHING,
    REBOOTING,
    OFFLINE,
};

enum system_timers {
    HEARTBEAT,
    SETTINGS,
    FIRMWARE,
    NTP,
    NEIGHBORDATA,
    SENSORREADING,
    MESSAGE,
    MAXSYSTEMTIMERS,
};

struct pid_gains {
    float K_p;
    float K_i;
    float K_d;
};

struct limits {
    float min;
    float max;
};

#endif  // SRC_HEADERS_DEFINITIONS_H_
