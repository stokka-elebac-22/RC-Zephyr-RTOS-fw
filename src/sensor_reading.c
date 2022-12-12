// "Copyright 2022 RC-project"
#include <zephyr.h>
#include <devicetree.h>
#include <drivers/sensor.h>
#include <logging/log.h>

#include "headers/definitions.h"
#include "headers/hcsr_distance_sensor.h"
LOG_MODULE_REGISTER(sensor, LOG_LEVEL_INF);

struct sensordata sensor_readings[MAX_SENSOR_READINGS];

#ifdef CONFIG_BME280
#define TEMP_SENSOR DT_LABEL(DT_NODELABEL(bme280))
const struct device *dev_env;
// struct sensor_value temp, press, humidity;
#endif   // BME280
#ifdef CONFIG_VL53L0X
const struct device *dev_vl5310x;
// struct sensor_value vl53_value;
#endif   // VL53L0x
#ifdef CONFIG_MPU6500

#endif   // MPU6500

int init_sensors() {
#ifdef CONFIG_BME280
    dev_env = DEVICE_DT_GET_ANY(bosch_bme280);
    if (dev_env == NULL) {
        LOG_ERR("Error: BME280 not found.");
    }
    if (!device_is_ready(dev_env)) {
        LOG_ERR("Device BME280 is not ready, check the driver initialization logs for errors.");
    }
    LOG_INF("Found device BME280 getting sensor data\n");
#endif

#ifdef CONFIG_VL53L0X
    dev_vl5310x = DEVICE_DT_GET_ANY(st_vl53l0x);
    if (dev_vl5310x == NULL) {
        LOG_ERR("Error: VL53L0x not found.");
    }
    if (!device_is_ready(dev_vl5310x)) {
        LOG_ERR("Device VL53L0x is not ready, check the driver initialization logs for errors.");
    }
    LOG_INF("Found device VL53L0x getting sensor data\n");
#endif   // VL53L0x

#ifdef CONFIG_MPU6500
    // const char *const label = DT_LABEL(DT_INST(0, invensense_mpu9250));
    // const struct device *mpu9250 = device_get_binding(label);
    // if (!mpu6050) {
    //     printf("Failed to find sensor %s\n", label);
    //     return;
    // }
#endif   // MPU6500

    return 0;
}

void work_make_readings_and_send(struct k_work *work) {
    uint8_t currentreading;
    int ret;
#ifdef CONFIG_BME280
    currentreading = BME280_AMBIENT_TEMP;
    if (dev_env != NULL) {
        LOG_DBG("Make environment reading");
        int channels[] = {SENSOR_CHAN_AMBIENT_TEMP, SENSOR_CHAN_PRESS, SENSOR_CHAN_HUMIDITY};
        ret = sensor_sample_fetch(dev_env);
        if (ret) {
            LOG_ERR("BME280 fetch failed ret: %d\n", ret);
        } else {
            for (int i = 0; i < (sizeof(channels) / sizeof(int)); i++) {
                if (currentreading < MAX_SENSOR_READINGS) {
                    sensor_readings[currentreading].measurement_type = channels[i];
                    sensor_channel_get(dev_env, channels[i], &sensor_readings[currentreading].data_channel);
                    currentreading++;
                } else {
                    LOG_ERR("Sensor buffer full");
                }
            }
        }
    }
#endif

#ifdef CONFIG_VL53L0X
    currentreading = VL53L0X_DISTANCE;
    if (dev_vl5310x != NULL) {
        LOG_DBG("Make distance reading");
        int channels[] = {SENSOR_CHAN_DISTANCE, SENSOR_CHAN_PROX};
        ret = sensor_sample_fetch(dev_vl5310x);
        if (ret) {
            LOG_ERR("VL53L0X fetch failed ret: %d\n", ret);
        } else {
            for (int i = 0; i < (sizeof(channels) / sizeof(int)); i++) {
                if (currentreading < MAX_SENSOR_READINGS) {
                    sensor_readings[currentreading].measurement_type = channels[i];
                    sensor_channel_get(dev_vl5310x, channels[i], &sensor_readings[currentreading].data_channel);
                    currentreading++;
                } else {
                    LOG_ERR("Sensor buffer full");
                }
            }
        }
    }
#endif
    currentreading = HCSR04_0;
    LOG_DBG("Make distance reading");
    int channels[] = {SENSOR_CHAN_DISTANCE, SENSOR_CHAN_DISTANCE};
    for (int i = 0; i < (sizeof(channels) / sizeof(int)); i++) {
        if (currentreading < MAX_SENSOR_READINGS) {
            sensor_readings[currentreading].measurement_type = channels[i];
            hcsr_get(i, channels[i], &sensor_readings[currentreading].data_channel);
            currentreading++;
        } else {
            LOG_ERR("Sensor buffer full");
        }
    }

#ifdef CONFIG_MPU6500

#endif   // MPU6500

#ifdef CONFIG_CAN
    // Transmit over CAN (?)
#endif   // CAN
}

K_WORK_DELAYABLE_DEFINE(make_rd_and_send, work_make_readings_and_send);

int make_reading_and_send() {
    k_work_schedule(&make_rd_and_send, K_MSEC(10));
    return 0;
}

void log_output_all_sensors() {
    #ifdef CONFIG_BME280
            LOG_INF("temp: %d.%02d; press: %d.%03d; humidity: %d.%03d",
                sensor_readings[BME280_AMBIENT_TEMP].data_channel.val1,
                sensor_readings[BME280_AMBIENT_TEMP].data_channel.val2,
                sensor_readings[BME280_PRESS].data_channel.val1,
                sensor_readings[BME280_PRESS].data_channel.val2,
                sensor_readings[BME280_HUMIDITY].data_channel.val1,
                sensor_readings[BME280_HUMIDITY].data_channel.val2);
    #endif
    #ifdef CONFIG_VL53L0X
            LOG_INF("Distance: %d.%02d; proximity: %d",
                sensor_readings[VL53L0X_DISTANCE].data_channel.val1,
                sensor_readings[VL53L0X_DISTANCE].data_channel.val2,
                sensor_readings[VL53L0X_PROX].data_channel.val1);
    #endif
    // if distance sensors
        LOG_INF("Distance HCSR04-0: %d.%02d",
            sensor_readings[HCSR04_0].data_channel.val1,
            sensor_readings[HCSR04_0].data_channel.val2);
        LOG_INF("Distance HCSR04-1: %d.%02d",
            sensor_readings[HCSR04_1].data_channel.val1,
            sensor_readings[HCSR04_1].data_channel.val2);
    // endif distance sensors
}
