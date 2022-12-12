/* sensor_as7341.c - driver for AS7341 11 Channel Spectral Color sensor */

/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT ams_as7341

#include <kernel.h>
#include <init.h>
#include <drivers/i2c.h>
#include <drivers/sensor.h>
#include <logging/log.h>

#include "h/as7341.h"

LOG_MODULE_REGISTER(AS7341, CONFIG_SENSOR_LOG_LEVEL);

struct as7341_config {
    struct i2c_dt_spec i2c;
};

static int as7341_sample_fetch(const struct device *dev,
                 enum sensor_channel chan) {
    const struct as7341_config *config = dev->config;
    struct as7341_driver_data *drv_data = dev->data;

    uint8_t buf[12];
    if (i2c_burst_read_dt(&config->i2c,
               AS7341_CH0_DATA_L, (uint8_t *)buf, 12) < 0) {
        LOG_DBG("Could not read color data");
        return -EIO;
    }
    drv_data->channel_f1_415nm_sample = (buf[0] << 8 | buf[1]);
    drv_data->channel_f2_445nm_sample = (buf[2] << 8 | buf[3]);
    drv_data->channel_f3_480nm_sample = (buf[4] << 8 | buf[5]);
    drv_data->channel_f4_515nm_sample = (buf[6] << 8 | buf[7]);
    drv_data->channel_clear = (buf[8] << 8 | buf[9]);
    drv_data->channel_nir = (buf[10] << 8 | buf[11]);
    // Setup SMUX for channel f5-f8 + clear + nir
    return 0;
}

static void as7341_channel_lux_convert(struct sensor_value *val,
                    int64_t raw_val) {
    /*
     * TODO: Create logic to convert meassurement to lux
     */
    val->val1 = raw_val;
}

static int as7341_channel_get(const struct device *dev,
                enum sensor_channel chan,
                struct sensor_value *val) {
    struct as7341_driver_data *drv_data = dev->data;
    /*
     * See datasheet "Sensor data" section for
     * more details on processing sample data.
     */
    if ((uint16_t)chan == SENSOR_CHAN_F1_415) {
        as7341_channel_lux_convert(val, drv_data->channel_f1_415nm_sample);
    } else if ((uint16_t)chan == SENSOR_CHAN_F2_445) {
        as7341_channel_lux_convert(val, drv_data->channel_f2_445nm_sample);
    } else if ((uint16_t)chan == SENSOR_CHAN_BLUE || SENSOR_CHAN_F3_480) {
        /** Illuminance in blue spectrum, in lux. */
        as7341_channel_lux_convert(val, drv_data->channel_f3_480nm_sample);
    } else if ((uint16_t)chan == SENSOR_CHAN_F4_515) {
        as7341_channel_lux_convert(val, drv_data->channel_f4_515nm_sample);
    } else if ((uint16_t)chan == SENSOR_CHAN_GREEN || SENSOR_CHAN_F5_555) {
        /** Illuminance in green spectrum, in lux. */
        as7341_channel_lux_convert(val, drv_data->channel_f5_555nm_sample);
    } else if ((uint16_t)chan == SENSOR_CHAN_F6_590) {
        as7341_channel_lux_convert(val, drv_data->channel_f6_590nm_sample);
    } else if ((uint16_t)chan == SENSOR_CHAN_F7_630) {
        as7341_channel_lux_convert(val, drv_data->channel_f7_630nm_sample);
    } else if ((uint16_t)chan == SENSOR_CHAN_RED || SENSOR_CHAN_F8_680) {
        /** Illuminance in red spectrum, in lux. */
        as7341_channel_lux_convert(val, drv_data->channel_f8_680nm_sample);
    } else if ((uint16_t)chan == SENSOR_CHAN_IR || SENSOR_CHAN_NIR) {
        /** Illuminance in infra-red spectrum, in lux. */
        as7341_channel_lux_convert(val, drv_data->channel_nir);
    } else if ((uint16_t)chan == SENSOR_CHAN_CLEAR) {
        as7341_channel_lux_convert(val, drv_data->channel_clear);
    } else if ((uint16_t)chan == SENSOR_CHAN_FD) {
        as7341_channel_lux_convert(val, drv_data->channel_fd);
    } else {
        return -ENOTSUP;
    }
    return 0;
}

static const struct sensor_driver_api as7341_api = {
    .sample_fetch = &as7341_sample_fetch,
    .channel_get = &as7341_channel_get,
};

static int as7341_init(const struct device *dev) {
    const struct as7341_config *config = dev->config;
     uint8_t id = 0U;

    // power management register 0X6B we should write all 0's to wake the sensor up
    if (!device_is_ready(config->i2c.bus)) {
        LOG_DBG("I2C bus is not ready");
        return -ENODEV;
    }

    // read device ID
    if (i2c_reg_read_byte_dt(&config->i2c,
                  AS7341_REG_CHIP_ID, &id) < 0) {
        LOG_DBG("Could not read chip id");
        return -EIO;
    }

    if (id != AS7341_DEFAULT_CHIP_ID) {
        LOG_DBG("Unexpected chip id (%x)", id);
        return -EIO;
    }
    if (i2c_reg_write_byte_dt(&config->i2c,
                AS7341_ENABLE, PON | SP_ON) < 0) {
        LOG_DBG("Fail to write power management register 0x80");
        return -EINVAL;
    }
    if (i2c_reg_write_byte_dt(&config->i2c,
                SPECRAT_CONFIG, 0x00) < 0) {
        LOG_DBG("Fail to write DATA to config register");
        return -EINVAL;
    }
    return 0;
}

struct as7341_driver_data as7341_data;
static const struct as7341_config as7341_config = {
    .i2c = I2C_DT_SPEC_INST_GET(0),
};

DEVICE_DT_INST_DEFINE(0, as7341_init, NULL,
            &as7341_data, &as7341_config, POST_KERNEL,
            CONFIG_SENSOR_INIT_PRIORITY, &as7341_api);

