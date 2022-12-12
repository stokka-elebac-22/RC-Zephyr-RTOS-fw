// "Copyright 2022 RC-project"
#include <zephyr.h>
#include <stdio.h>
#include <device.h>
#include <drivers/gpio.h>
#include <drivers/sensor.h>
#include <logging/log.h>

#include "headers/definitions.h"
#include "headers/hcsr_distance_sensor.h"


LOG_MODULE_REGISTER(distance, LOG_LEVEL_INF);

// USER BTN Interrupt:
#define TRIG_PIN_0_NODE	DT_ALIAS(trig0)
#define ECHO_PIN_0_NODE	DT_ALIAS(echo0)
#define TRIG_PIN_1_NODE	DT_ALIAS(trig1)
#define ECHO_PIN_1_NODE	DT_ALIAS(echo1)
// #define TRIG_0_PIN DT_GPIO_PIN(TRIG_PIN_0_NODE, gpios)
// #define TRIG_1_PIN DT_GPIO_PIN(TRIG_PIN_1_NODE, gpios)

#if !DT_NODE_HAS_STATUS(TRIG_PIN_0_NODE, okay)
#error "Unsupported board: trig0 devicetree alias is not defined"
#endif
#if !DT_NODE_HAS_STATUS(ECHO_PIN_0_NODE, okay)
#error "Unsupported board: echo0 devicetree alias is not defined"
#endif
#if !DT_NODE_HAS_STATUS(TRIG_PIN_1_NODE, okay)
#error "Unsupported board: trig1 devicetree alias is not defined"
#endif
#if !DT_NODE_HAS_STATUS(ECHO_PIN_1_NODE, okay)
#error "Unsupported board: echo1 devicetree alias is not defined"
#endif

#define TRIG_TIME K_USEC(10)
#define MAX_ECHO_WAIT 0
#define SENSE_FREQ K_MSEC(400)


/* Timings defined by spec */
#define T_TRIG_PULSE_US     K_USEC(11)
#define T_INVALID_PULSE_US    25000
#define T_MAX_WAIT_MS         130
#define T_SPURIOS_WAIT_US     145
#define METERS_PER_SEC        340


#define DIST_SENSOR_FACTOR METERS_PER_SEC / 2

uint64_t trig_time = 0;
uint8_t cur_dist_sensor = 0;
uint8_t trig_type = 0;

static const struct gpio_dt_spec trig_0 = GPIO_DT_SPEC_GET_OR(TRIG_PIN_0_NODE, gpios, {0});
static const struct gpio_dt_spec trig_1 = GPIO_DT_SPEC_GET_OR(TRIG_PIN_1_NODE, gpios, {0});
static const struct gpio_dt_spec echo_0 = GPIO_DT_SPEC_GET_OR(ECHO_PIN_0_NODE, gpios, {0});
static const struct gpio_dt_spec echo_1 = GPIO_DT_SPEC_GET_OR(ECHO_PIN_1_NODE, gpios, {0});

static struct gpio_callback echo_0_cb_data;
static struct gpio_callback echo_1_cb_data;

struct sensor_value distance_readings[MAX_DISTANCE_SENSORS];

void hcsr_timing_handler(struct k_timer *dummy) {
    // Switch between the two sensors ...
    cur_dist_sensor = !cur_dist_sensor;  // 0 or 1 ...
    LOG_DBG("Sending trigger on %d ...", cur_dist_sensor);
    send_trig();
}

K_TIMER_DEFINE(hcsr_timer, hcsr_timing_handler, NULL);

void hcsr_get(int sensor_no, int channel, struct sensor_value *sv) {
    sv->val1 = distance_readings[sensor_no].val1;
    sv->val2 = distance_readings[sensor_no].val2;
}

void echo_returned(const struct device *dev, struct gpio_callback *cb, uint32_t pins) {
    if (trig_type == 0) {
        if (cur_dist_sensor == 0) {
            gpio_remove_callback(echo_0.port, &echo_0_cb_data);
            gpio_pin_interrupt_configure_dt(&echo_0, GPIO_INT_EDGE_FALLING);
            gpio_add_callback(echo_0.port, &echo_0_cb_data);
        } else {
            gpio_remove_callback(echo_1.port, &echo_1_cb_data);
            gpio_pin_interrupt_configure_dt(&echo_1, GPIO_INT_EDGE_FALLING);
            gpio_add_callback(echo_1.port, &echo_1_cb_data);
        }
        trig_time = k_cycle_get_32();
        trig_type = 1;
    } else {
        uint64_t diff = k_cyc_to_us_floor64(k_cycle_get_32() - trig_time);
        int distance = diff * DIST_SENSOR_FACTOR;
        distance_readings[cur_dist_sensor].val2 = (distance % 1000000);
        distance_readings[cur_dist_sensor].val1 = (distance / 1000000);
    }
}

void hcsr_init() {
    LOG_INF("Init HC-SR04 distance sensors");
    gpio_pin_configure_dt(&trig_0, GPIO_OUTPUT);
    gpio_pin_configure_dt(&trig_1, GPIO_OUTPUT);
	gpio_init_callback(&echo_0_cb_data, echo_returned, BIT(echo_0.pin));
	gpio_init_callback(&echo_1_cb_data, echo_returned, BIT(echo_1.pin));
	int ret;
	if (!device_is_ready(echo_0.port)) {
		LOG_ERR("Error: Echo-pin device %s is not ready\n",
		       echo_0.port->name);
		return;
	}
	ret = gpio_pin_configure_dt(&echo_0, GPIO_INPUT);
	if (ret != 0) {
		LOG_ERR("Error %d: failed to configure %s pin %d\n",
		       ret, echo_0.port->name, echo_0.pin);
		return;
	}
	ret = gpio_pin_interrupt_configure_dt(&echo_0, GPIO_INT_EDGE_TO_ACTIVE);
	if (ret != 0) {
		LOG_ERR("Error %d: failed to configure interrupt on %s pin %d\n",
			ret, echo_0.port->name, echo_0.pin);
		return;
	}
	if (!device_is_ready(echo_1.port)) {
		LOG_ERR("Error: Echo-pin device %s is not ready\n",
		       echo_1.port->name);
		return;
	}
	ret = gpio_pin_configure_dt(&echo_1, GPIO_INPUT);
	if (ret != 0) {
		LOG_ERR("Error %d: failed to configure %s pin %d\n",
		       ret, echo_1.port->name, echo_1.pin);
		return;
	}
	ret = gpio_pin_interrupt_configure_dt(&echo_1, GPIO_INT_EDGE_TO_ACTIVE);
	if (ret != 0) {
		LOG_ERR("Error %d: failed to configure interrupt on %s pin %d\n",
			ret, echo_1.port->name, echo_1.pin);
		return;
	}
    k_timer_start(&hcsr_timer, SENSE_FREQ, SENSE_FREQ);
}

void send_trig() {
    trig_time = k_cycle_get_32();
    if (!cur_dist_sensor) {
        gpio_pin_set_dt(&trig_0, 1);
        k_sleep(T_TRIG_PULSE_US);
        gpio_pin_set_dt(&trig_0, 0);
        gpio_add_callback(echo_0.port, &echo_0_cb_data);
        LOG_DBG("Trig sent on %s pin %d", echo_0.port->name, echo_0.pin);
    } else {
        gpio_pin_set_dt(&trig_1, 1);
        k_sleep(T_TRIG_PULSE_US);
        gpio_pin_set_dt(&trig_1, 0);
        gpio_add_callback(echo_1.port, &echo_1_cb_data);
        gpio_pin_interrupt_configure_dt(&echo_1, GPIO_INT_EDGE_RISING);
        trig_type = 0;
        LOG_DBG("Trig sent on %s pin %d", echo_1.port->name, echo_1.pin);
    }
}
