/*
 * Copyright (c) 2022
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <device.h>
#include <drivers/gpio.h>
#include <drivers/sensor.h>
#include <drivers/uart.h>
#include <logging/log.h>

#include <stdio.h>
#include <sys/printk.h>
#include <usb/usb_device.h>

// #include <time.h>
// #include <stdlib.h>
// #include <errno.h>
// #include <unistd.h>

#ifdef CONFIG_IMG_MANAGER
#include <dfu/mcuboot.h>
#include <dfu/flash_img.h>
#endif

#include "headers/pwm_control.h"
#include "headers/shell_menu.h"
#include "headers/git.h"
#include "headers/sensor_reading.h"
#include "headers/hcsr_distance_sensor.h"


#define WAIT_UNTIL_USB_CONNECTION
#define WQ_TIME_MS 100
#define PID_INTERVAL 1

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

// USER BTN Interrupt:
#define SW0_NODE    DT_ALIAS(sw0)
#if !DT_NODE_HAS_STATUS(SW0_NODE, okay)
#error "Unsupported board: sw0 devicetree alias is not defined"
#endif
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET_OR(SW0_NODE, gpios, {0});
static struct gpio_callback button_cb_data;

void button_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins) {
    LOG_INF("Button pressed at %" PRIu32 "\n", k_cycle_get_32());
    // Can be used to reset distance / change mode ...
    // gpio_remove_callback(button.port, &button_cb_data);
}

void init_button_callback() {
    int ret;
    if (!device_is_ready(button.port)) {
        LOG_ERR("Error: button device %s is not ready\n",
               button.port->name);
        return;
    }

    ret = gpio_pin_configure_dt(&button, GPIO_INPUT);
    if (ret != 0) {
        LOG_ERR("Error %d: failed to configure %s pin %d\n",
               ret, button.port->name, button.pin);
        return;
    }

    ret = gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_TO_ACTIVE);
    if (ret != 0) {
        LOG_ERR("Error %d: failed to configure interrupt on %s pin %d\n",
            ret, button.port->name, button.pin);
        return;
    }

    gpio_init_callback(&button_cb_data, button_pressed, BIT(button.pin));
    gpio_add_callback(button.port, &button_cb_data);
    LOG_INF("Set up button at %s pin %d\n", button.port->name, button.pin);
}

////////////////////  WORKQUEUE  /////////////////
// Will run stuff periodicaly
static uint32_t wq_counter = 0;
static uint32_t last_run_pid = 0;

void wq_timing_handler(struct k_timer *dummy) {
    if (wq_counter >= last_run_pid + PID_INTERVAL) {
         // run PID functionality or set flag to do so in main loop;
         last_run_pid = wq_counter;
    }
    wq_counter += WQ_TIME_MS;
}

K_TIMER_DEFINE(wq_timer, wq_timing_handler, NULL);
/* Start workque timer */
int wq_init() {
    k_timer_start(&wq_timer, K_MSEC(WQ_TIME_MS), K_MSEC(WQ_TIME_MS));
    return 0;
}

void main(void) {
    if (usb_enable(NULL)) {
        LOG_ERR("Failed to enable USB");
        return;
    }
    /* Poll if the DTR flag was set, wait until connected ... */
    #ifdef WAIT_UNTIL_USB_CONNECTION
    uint32_t dtr = 0;
    const struct device *dev_console = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));
    while (!dtr) {
        uart_line_ctrl_get(dev_console, UART_LINE_CTRL_DTR, &dtr);
        k_sleep(K_MSEC(1000));
    }
    #endif  // WAIT UNTIL USB CONNECTION

    LOG_INF("ELEBAC22 RC - %s \n", GIT_DESCRIBE);
    LOG_INF("HC-SR04 initializing!");
    hcsr_init();  // Initialize HC-SR04 sensors
    LOG_INF("Sensor handler initializing!");
    init_sensors();
    LOG_INF("Motor PWM initializing!");
    motor_init();
    LOG_INF("Button callback initializing!");
    init_button_callback();
    // LOG_INF("Setting handler initializing!");
    // settings_init();
    LOG_INF("Work queue for PID etc initializing!");
    wq_init();

    while (1) {
        make_reading_and_send();
        /* Give CPU resources to low priority threads. */
        k_sleep(K_MSEC(100));
        log_output_all_sensors();
        k_sleep(K_MSEC(900));
    }
}
