/*
 * Copyright (c) 2020 Seagate Technology LLC
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT pwm_leds

/**
 * @file
 * @brief PWM driven LEDs
 */

#include <drivers/led.h>
#include <drivers/pwm.h>
#include <device.h>
#include <zephyr.h>
#include <sys/math_extras.h>

#include <logging/log.h>
LOG_MODULE_REGISTER(led_slowpwm, CONFIG_LED_LOG_LEVEL);
#include "./led_context.h"

#define DEV_CFG(dev)    ((const struct led_slowpwm_config *) ((dev)->config))

struct led_slowpwm {
    const struct device *dev;
    uint32_t channel;
    uint32_t period;
    pwm_flags_t flags;
};

struct led_slowpwm_config {
    int num_leds;
    const struct led_slowpwm *led;
};

static int led_slowpwm_blink(const struct device *dev, uint32_t led,
             uint32_t delay_on, uint32_t delay_off) {
    const struct led_slowpwm_config *config = DEV_CFG(dev);
    const struct led_slowpwm *led_slowpwm;
    uint32_t period_usec, pulse_usec;

    if (led >= config->num_leds) {
        return -EINVAL;
    }

    /*
     * Convert delays (in ms) into PWM period and pulse (in us) and check
     * for overflows.
     */
    if (u32_add_overflow(delay_on, delay_off, &period_usec) ||
        u32_mul_overflow(period_usec, 1000, &period_usec) ||
        u32_mul_overflow(delay_on, 1000, &pulse_usec)) {
        return -EINVAL;
    }

    led_slowpwm = &config->led[led];

    return pwm_pin_set_usec(led_slowpwm->dev, led_slowpwm->channel,
                period_usec, pulse_usec, led_slowpwm->flags);
}

static int led_slowpwm_set_brightness(const struct device *dev,
                  uint32_t led, uint8_t value) {
    const struct led_slowpwm_config *config = DEV_CFG(dev);
    const struct led_slowpwm *led_slowpwm;
    uint32_t pulse;

    if (led >= config->num_leds || value > 100) {
        return -EINVAL;
    }

    led_slowpwm = &config->led[led];

    pulse = led_slowpwm->period * value / 100;

    return pwm_pin_set_nsec(led_slowpwm->dev, led_slowpwm->channel,
                  led_slowpwm->period, pulse, led_slowpwm->flags);
}

static int led_slowpwm_on(const struct device *dev, uint32_t led) {
    return led_slowpwm_set_brightness(dev, led, 100);
}

static int led_slowpwm_off(const struct device *dev, uint32_t led) {
    return led_slowpwm_set_brightness(dev, led, 0);
}

static int led_slowpwm_init(const struct device *dev) {
    const struct led_slowpwm_config *config = DEV_CFG(dev);
    int i;

    if (!config->num_leds) {
        LOG_ERR("%s: no LEDs found (DT child nodes missing)",
            dev->name);
        return -ENODEV;
    }

    for (i = 0; i < config->num_leds; i++) {
        const struct led_slowpwm *led = &config->led[i];

        if (!device_is_ready(led->dev)) {
            LOG_ERR("%s: pwm device not ready", dev->name);
            return -ENODEV;
        }
    }

    return 0;
}

#ifdef CONFIG_PM_DEVICE
static int led_slowpwm_pm_control(const struct device *dev,
                  enum pm_device_action action) {
    const struct led_slowpwm_config *config = DEV_CFG(dev);

    /* switch all underlying PWM devices to the new state */
    for (size_t i = 0; i < config->num_leds; i++) {
        int err;
        enum pm_device_state state;
        const struct led_slowpwm *led_slowpwm = &config->led[i];

        LOG_DBG("Switching PWM %p to state %" PRIu32, led_slowpwm->dev, state);

        /* NOTE: temporary solution, deserves proper fix */
        switch (action) {
        case PM_DEVICE_ACTION_RESUME:
            state = PM_DEVICE_STATE_ACTIVE;
            break;
        case PM_DEVICE_ACTION_SUSPEND:
            state = PM_DEVICE_STATE_SUSPENDED;
            break;
        default:
            return -ENOTSUP;
        }

        err = pm_device_state_set(led_slowpwm->dev, state);
        if (err) {
            LOG_ERR("Cannot switch PWM %p power state", led_slowpwm->dev);
        }
    }

    return 0;
}
#endif /* CONFIG_PM_DEVICE */

static const struct led_driver_api led_slowpwm_api = {
    .on        = led_slowpwm_on,
    .off        = led_slowpwm_off,
    .blink        = led_slowpwm_blink,
    .set_brightness    = led_slowpwm_set_brightness,
};

#define LED_SLOWPWM(led_node_id) { \
    .dev        = DEVICE_DT_GET(DT_PWMS_CTLR(led_node_id)),    \
    .channel    = DT_PWMS_CHANNEL(led_node_id),            \
    .period        = DT_PHA_OR(led_node_id, pwms, period, 1000000),    \
    .flags        = DT_PHA_OR(led_node_id, pwms, flags,        \
                    PWM_POLARITY_NORMAL),        \
},

#define LED_SLOWPWM_DEVICE(id)                    \
                                \
static const struct led_slowpwm led_slowpwm_##id[] = {            \
    DT_INST_FOREACH_CHILD(id, LED_SLOWPWM)            \
};                                \
                                \
static const struct led_slowpwm_config led_slowpwm_config_##id = {    \
    .num_leds    = ARRAY_SIZE(led_slowpwm_##id),        \
    .led        = led_slowpwm_##id,                \
};                                \
                                \
DEVICE_DT_INST_DEFINE(id, &led_slowpwm_init, led_slowpwm_pm_control,    \
              NULL, &led_slowpwm_config_##id, POST_KERNEL,    \
              CONFIG_LED_INIT_PRIORITY, &led_slowpwm_api);

DT_INST_FOREACH_STATUS_OKAY(LED_SLOWPWM_DEVICE)
