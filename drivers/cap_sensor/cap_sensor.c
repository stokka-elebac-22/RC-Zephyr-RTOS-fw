/* sensor_cap_sensor.c - driver for cap_sensor */
/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT cap_sensor

#include <kernel.h>
#include <init.h>
#include <drivers/pwm.h>
#include <drivers/sensor.h>
#include <drivers/adc.h>
#include <drivers/gpio.h>
#include <logging/log.h>
#ifdef CONFIG_ADC_NRFX_SAADC
#include <hal/nrf_saadc.h>
#endif

LOG_MODULE_REGISTER(cap_sensor, CONFIG_SENSOR_LOG_LEVEL);

struct cap_sensor_config {
    const struct device *adc;
    const struct device *pwm;
    const struct gpio_dt_spec gpio;
    struct adc_channel_cfg cap_cfg;
    struct adc_channel_cfg vdd_cfg;
    uint32_t pwm_channel;
    uint32_t pwm_period;
    pwm_flags_t pwm_flags;
    uint8_t gpio_pin;
    uint8_t gpio_flags;
};
struct cap_sensor_data {
    int16_t raw_cap;
    int16_t raw_vdd;
    int32_t millis_cap;
    int32_t millis_vdd;
    struct adc_sequence adc_seq_cap;
    struct adc_sequence adc_seq_vdd;
};
enum pwmaction {
    StartPwm,
    StopPwm,
};
int soilsensor_pwm(const struct cap_sensor_config *config, enum pwmaction action) {
    int ret = 0;
    switch (action) {
    case StartPwm:
        pm_device_state_set(config->pwm, PM_DEVICE_STATE_ACTIVE);
        gpio_pin_configure_dt(&config->gpio, GPIO_OUTPUT_ACTIVE);
        gpio_pin_set_dt(&config->gpio, 1);
        ret = pwm_pin_set_usec(config->pwm, config->pwm_channel,
        config->pwm_period, (config->pwm_period / 2U), config->pwm_flags);
        break;
    case StopPwm:
        gpio_pin_set_dt(&config->gpio, 0);
        gpio_pin_configure_dt(&config->gpio, GPIO_DISCONNECTED);
        ret = pwm_pin_set_usec(config->pwm, config->pwm_channel,
            0, 0, config->pwm_flags);
        pm_device_state_set(config->pwm, PM_DEVICE_STATE_SUSPENDED);
        break;
    default:
        break;
    }
    return ret;
}

static inline double get_soil_moisture_percent(double vdd, double raw_adc_output) {
    const double dry = -12.9 * vdd * vdd + 111 * vdd + 228;
    const double wet = -5.71 * vdd * vdd + 60.2 * vdd + 126;
    LOG_DBG("[adc] batt: %lf cap: %lf", vdd, raw_adc_output);
    LOG_DBG("[adc] dry: %lf wet: %lf", dry, wet);
    double value = (raw_adc_output - dry) / (wet - dry);
    return (value > 1.0 ? 1.0 : (value < 0.0 ? 0.0 : value)) * 100.0;
}
static int cap_sensor_sample_fetch(const struct device *dev, enum sensor_channel chan) {
    int rc = 0;
    const struct cap_sensor_config *config = dev->config;
    const struct adc_channel_cfg *acpv = &config->vdd_cfg;
    // const struct adc_channel_cfg *acpc = &config->cap_cfg;
    struct cap_sensor_data *data = dev->data;
    struct adc_sequence *adc_seqv = &data->adc_seq_vdd;
    struct adc_sequence *adc_seqc = &data->adc_seq_cap;
    pm_device_state_set(config->adc, PM_DEVICE_STATE_ACTIVE);
    adc_channel_setup(config->adc, &config->vdd_cfg);
    adc_channel_setup(config->adc, &config->cap_cfg);
    soilsensor_pwm(config, StartPwm);
    if (!device_is_ready(config->adc)) {
        LOG_ERR("ADC device is not ready %s", config->adc->name);
        return -ENOENT;
    }
    if (rc) {
        LOG_INF("Error setting up ADC got %d", rc);
        return -ENOTSUP;
    }
    rc = adc_read(config->adc, adc_seqv);
    rc = adc_read(config->adc, adc_seqc);
    data->millis_cap = data->raw_cap;
    data->millis_vdd = data->raw_vdd;
    adc_raw_to_millivolts(adc_ref_internal(config->adc), acpv->gain, adc_seqv->resolution, &data->millis_vdd);
    soilsensor_pwm(config, StopPwm);
    pm_device_state_set(config->adc, PM_DEVICE_STATE_SUSPENDED);
    return rc;
}


static int cap_sensor_channel_get(const struct device *dev, enum sensor_channel chan, struct sensor_value *val) {
    const struct cap_sensor_data *data = dev->data;
    if ((uint16_t)chan == SENSOR_CHAN_VOLTAGE) {
        val->val1 = data->millis_vdd / 1000U;
        val->val2 = (data->millis_vdd % 1000U) * 1000;
    } else if ((uint16_t)chan == SENSOR_CHAN_HUMIDITY) {
        double vdd = data->millis_vdd / 1000.0;
        double cap = data->raw_cap;
        double moist = get_soil_moisture_percent(vdd, cap);
        LOG_DBG("[moist]: %lf", moist);
        val->val1 = (int)moist;
        val->val2 = (moist - (int)moist) * 1000000U;
    } else {
        return -ENOTSUP;
    }
    return 0;
}

static const struct sensor_driver_api cap_sensor_api = {
    .sample_fetch = &cap_sensor_sample_fetch,
    .channel_get = &cap_sensor_channel_get,
};

static int cap_sensor_init(const struct device *dev) {
    const struct cap_sensor_config *config = dev->config;
    struct cap_sensor_data *data = dev->data;
    struct adc_sequence *aspv = &data->adc_seq_vdd;
    struct adc_sequence *aspc = &data->adc_seq_cap;
    if (!device_is_ready(config->adc)) {
        LOG_DBG("ADC dev is not ready");
        return -ENODEV;
    }
    #ifdef CONFIG_ADC_NRFX_SAADC
    *aspv = (struct adc_sequence){
        .channels = BIT(1),
        .buffer = &data->raw_vdd,
        .buffer_size = sizeof(data->raw_vdd),
        .resolution  = 10,
        .oversampling = 2,
        .calibrate = false,
    };
    *aspc = (struct adc_sequence){
        .channels = BIT(0),
        .buffer = &data->raw_cap,
        .buffer_size = sizeof(data->raw_cap),
        .resolution  = 10,
        .oversampling = 2,
        .calibrate = false,
    };
    #endif /* CONFIG_ADC_NRFX_SAADC */
    return 0;
}

struct cap_sensor_data cap_sensor_data;
static const struct cap_sensor_config cap_sensor_config = {
    .pwm = DEVICE_DT_GET(DT_PHANDLE(DT_DRV_INST(0), pwm_dev)),
    .adc = DEVICE_DT_GET(DT_PHANDLE(DT_DRV_INST(0), adc_dev)),
    .gpio = GPIO_DT_SPEC_GET(DT_DRV_INST(0), gpios),
    // DEVICE_DT_GET(DT_PHANDLE(DT_DRV_INST(0), gpio_devs)),
    // .gpio_pin = DT_INST_GPIO_PIN(0, gpio_devs),
    // .gpio_flags = DT_INST_GPIO_FLAGS(0, gpio_devs),
    .pwm_channel = 5,
    .pwm_period = 2,
    .pwm_flags = PWM_POLARITY_NORMAL,
    .vdd_cfg = (struct adc_channel_cfg){
        .gain = ADC_GAIN_1_6,
        .reference = ADC_REF_INTERNAL,
        .channel_id = 1,
        .acquisition_time = ADC_ACQ_TIME(ADC_ACQ_TIME_MICROSECONDS, 40),
        .input_positive = NRF_SAADC_INPUT_VDD,
    },
    .cap_cfg = (struct adc_channel_cfg){
        .gain = ADC_GAIN_1_6,
        .reference = ADC_REF_VDD_1_4,
        .channel_id = 0,
        .acquisition_time = ADC_ACQ_TIME(ADC_ACQ_TIME_MICROSECONDS, 40),
        .input_positive = NRF_SAADC_INPUT_AIN1,
    },
};

DEVICE_DT_INST_DEFINE(0, cap_sensor_init, NULL,
            &cap_sensor_data, &cap_sensor_config, POST_KERNEL,
            CONFIG_SENSOR_INIT_PRIORITY, &cap_sensor_api);
