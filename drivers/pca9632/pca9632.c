/*
 * Copyright (c) 2018 Savoir-Faire Linux.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT nxp_pca9632

/**
 * @file
 * @brief LED driver for the PCA9632 I2C LED driver (7-bit slave address 0x62)
 */

#include <drivers/i2c.h>
#include <drivers/led.h>
#include <sys/util.h>
#include <zephyr.h>

#define LOG_LEVEL CONFIG_LED_LOG_LEVEL
#include <logging/log.h>
LOG_MODULE_REGISTER(pca9632);

#include "./led_context.h"

/* PCA9632 select registers determine the source that drives LED outputs */
#define PCA9632_LED_OFF         0x0     /* LED driver off */
#define PCA9632_LED_ON          0x1     /* LED driver on */
#define PCA9632_LED_PWM         0x2     /* Controlled through PWM */
#define PCA9632_LED_GRP_PWM     0x3     /* Controlled through PWM/GRPPWM */

/* PCA9632 control register */
#define PCA9632_MODE1           0x00
#define PCA9632_MODE2           0x01
#define PCA9632_PWM_BASE        0x02    /* Reg 0x02-0x05 for brightness control LED01-04 */
#define PCA9632_GRPPWM          0x06
#define PCA9632_GRPFREQ         0x07
#define PCA9632_LEDOUT          0x08

/* PCA9632 mode register 1 */
#define PCA9632_MODE1_SLEEP     0x10    /* Sleep Mode */
/* PCA9632 mode register 2 */
#define PCA9632_MODE2_DMBLNK    0x20    /* Enable blinking */
#define PCA9632_MODE2_OUTDRV    0x04    /* Set open-drain or totem pole structure. */
#define PCA9632_MODE2_INVRT        0x10

#define PCA9632_MASK            0x03

struct pca9632_config {
    struct i2c_dt_spec i2c;
};

struct pca9632_data {
    struct led_data dev_data;
};

static int pca9632_led_blink(const struct device *dev, uint32_t led,
                 uint32_t delay_on, uint32_t delay_off) {
    struct pca9632_data *data = dev->data;
    const struct pca9632_config *config = dev->config;
    struct led_data *dev_data = &data->dev_data;
    uint8_t gdc, gfrq;
    uint32_t period;

    period = delay_on + delay_off;

    if (period < dev_data->min_period || period > dev_data->max_period) {
        return -EINVAL;
    }

    /*
     * From manual:
     * duty cycle = (GDC / 256) ->
     *    (time_on / period) = (GDC / 256) ->
     *        GDC = ((time_on * 256) / period)
     */
    gdc = delay_on * 256U / period;
    if (i2c_reg_write_byte_dt(&config->i2c,
                   PCA9632_GRPPWM,
                   gdc)) {
        LOG_ERR("LED reg write failed");
        return -EIO;
    }

    /*
     * From manual:
     * period = ((GFRQ + 1) / 24) in seconds.
     * So, period (in ms) = (((GFRQ + 1) / 24) * 1000) ->
     *        GFRQ = ((period * 24 / 1000) - 1)
     */
    gfrq = (period * 24U / 1000) - 1;
    if (i2c_reg_write_byte_dt(&config->i2c,
                   PCA9632_GRPFREQ,
                   gfrq)) {
        LOG_ERR("LED reg write failed");
        return -EIO;
    }

    /* Enable blinking mode */
    if (i2c_reg_update_byte_dt(&config->i2c,
                PCA9632_MODE2,
                PCA9632_MODE2_DMBLNK,
                PCA9632_MODE2_DMBLNK)) {
        LOG_ERR("LED reg update failed");
        return -EIO;
    }

    /* Select the GRPPWM source to drive the LED outpout */
    if (i2c_reg_update_byte_dt(&config->i2c,
                PCA9632_LEDOUT,
                PCA9632_MASK << (led << 1),
                PCA9632_LED_GRP_PWM << (led << 1))) {
        LOG_ERR("LED reg update failed");
        return -EIO;
    }

    return 0;
}

static int pca9632_led_set_brightness(const struct device *dev, uint32_t led,
                      uint8_t value) {
    const struct pca9632_config *config = dev->config;
    struct pca9632_data *data = dev->data;
    struct led_data *dev_data = &data->dev_data;
    uint8_t val;

    if (value < dev_data->min_brightness ||
        value > dev_data->max_brightness) {
        return -EINVAL;
    }

    /* Set the LED brightness value */
    val = (value * 255U) / dev_data->max_brightness;
    if (i2c_reg_write_byte_dt(&config->i2c,
                   PCA9632_PWM_BASE + led,
                   val)) {
        LOG_ERR("LED reg write failed");
        return -EIO;
    }

    /* Set the LED driver to be controlled through its PWMx register. */
    if (i2c_reg_update_byte_dt(&config->i2c,
                PCA9632_LEDOUT,
                PCA9632_MASK << (led << 1),
                PCA9632_LED_PWM << (led << 1))) {
        LOG_ERR("LED reg update failed");
        return -EIO;
    }

    return 0;
}

static inline int pca9632_led_on(const struct device *dev, uint32_t led) {
    const struct pca9632_config *config = dev->config;

    /* Set LED state to ON */
    if (i2c_reg_update_byte_dt(&config->i2c,
                PCA9632_LEDOUT,
                PCA9632_MASK << (led << 1),
                PCA9632_LED_ON << (led << 1))) {
        LOG_ERR("LED reg update failed");
        return -EIO;
    }

    return 0;
}

static inline int pca9632_led_off(const struct device *dev, uint32_t led) {
    const struct pca9632_config *config = dev->config;

    /* Set LED state to OFF */
    if (i2c_reg_update_byte_dt(&config->i2c,
                PCA9632_LEDOUT,
                PCA9632_MASK << (led << 1),
                PCA9632_LED_OFF)) {
        LOG_ERR("LED reg update failed");
        return -EIO;
    }

    return 0;
}

static int pca9632_led_init(const struct device *dev) {
    const struct pca9632_config *config = dev->config;
    struct pca9632_data *data = dev->data;
    struct led_data *dev_data = &data->dev_data;

    if (!device_is_ready(config->i2c.bus)) {
        LOG_ERR("I2C bus is not ready");
        return -ENODEV;
    }

    /* Take the LED driver out from Sleep mode. */
    if (i2c_reg_update_byte_dt(&config->i2c,
                PCA9632_MODE1,
                PCA9632_MODE1_SLEEP,
                ~PCA9632_MODE1_SLEEP)) {
        LOG_ERR("LED reg update failed");
        return -EIO;
    }

    if (i2c_reg_update_byte_dt(&config->i2c,
                PCA9632_MODE2,
                0x14,
                (PCA9632_MODE2_OUTDRV | PCA9632_MODE2_INVRT))) {
        LOG_ERR("LED reg update failed");
        return -EIO;
    }
    /* Hardware specific limits */
    dev_data->min_period = 41U;
    dev_data->max_period = 10667U;
    dev_data->min_brightness = 0U;
    dev_data->max_brightness = 100U;

    return 0;
}

static const struct led_driver_api pca9632_led_api = {
    .blink = pca9632_led_blink,
    .set_brightness = pca9632_led_set_brightness,
    .on = pca9632_led_on,
    .off = pca9632_led_off,
};

#define PCA9632_DEVICE(id)                        \
    static const struct pca9632_config pca9632_##id##_cfg = {    \
        .i2c = I2C_DT_SPEC_INST_GET(id)                \
    };                                \
    static struct pca9632_data pca9632_##id##_data;            \
                                    \
    DEVICE_DT_INST_DEFINE(id, &pca9632_led_init, NULL,        \
            &pca9632_##id##_data,                \
            &pca9632_##id##_cfg, POST_KERNEL,        \
            CONFIG_LED_INIT_PRIORITY,            \
            &pca9632_led_api);

DT_INST_FOREACH_STATUS_OKAY(PCA9632_DEVICE)
