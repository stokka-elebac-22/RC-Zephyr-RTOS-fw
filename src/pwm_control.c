/*
 * Copyright (c) 2022
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <device.h>
#include <drivers/pwm.h>
#include <logging/log.h>
#include <stdio.h>
#include <sys/printk.h>

#include "headers/definitions.h"

LOG_MODULE_REGISTER(pwm_control, LOG_LEVEL_INF);

/* period of servo motor signal ->  20ms (50Hz) */
#define PERIOD (USEC_PER_SEC / 50U)

/* all in micro second */
#define STEP 100    /* PWM pulse step */
#define MINPULSEWIDTH 1000  /* Servo 0 degrees */
#define MIDDLEPULSEWIDTH 1500   /* Servo 90 degrees */
#define MAXPULSEWIDTH 2000  /* Servo 180 degrees */

#define HZ_TO_USEC(frequency)   (USEC_PER_SEC / frequency)

#define linmot_max_pwm 20000

#define MOTOR_0_0_PWM_PORT DT_PROP(DT_ALIAS(pwm1), ch0_pin)
#define MOTOR_0_1_PWM_PORT DT_PROP(DT_ALIAS(pwm1), ch1_pin)
#define MOTOR_1_0_PWM_PORT DT_PROP(DT_ALIAS(pwm3), ch0_pin)
#define MOTOR_1_1_PWM_PORT DT_PROP(DT_ALIAS(pwm3), ch1_pin)
#define MOTOR_EXT_PWM_PORT DT_PROP(DT_ALIAS(pwm0), ch0_pin)
#define SERVO_PWM_PORT DT_PROP(DT_ALIAS(pwm1), ch2_pin)
#define PWM0_DEV_NAME DT_ALIAS(pwm0)
#define PWM1_DEV_NAME DT_ALIAS(pwm1)
#define PWM3_DEV_NAME DT_ALIAS(pwm3)

const struct device *pwm0_dev;
const struct device *pwm1_dev;
const struct device *pwm3_dev;

int motor_init() {
    pwm0_dev = DEVICE_DT_GET(PWM0_DEV_NAME);
    if (!device_is_ready(pwm0_dev)) {
        LOG_ERR("PWM device is not ready\n");
    }
    pwm1_dev = DEVICE_DT_GET(PWM1_DEV_NAME);
    if (!device_is_ready(pwm1_dev)) {
        LOG_ERR("PWM device is not ready\n");
    }
    pwm3_dev = DEVICE_DT_GET(PWM3_DEV_NAME);
    if (!device_is_ready(pwm3_dev)) {
        LOG_ERR("PWM device is not ready\n");
    }
    return 0;
}

int motor_set_speed(enum motor_ids motor, int dir, int speed) {
    int speed_value = 200 - speed;
    switch (motor) {
        case MOTOR_LEFT:
            if (dir == DIR_FORWARD) {
                pwm_pin_set_cycles(pwm3_dev, MOTOR_1_0_PWM_PORT, 200, speed_value, 0);  // V FREM
                pwm_pin_set_cycles(pwm3_dev, MOTOR_1_1_PWM_PORT, 200, 200, 0);  // V FREM
            } else if (dir == DIR_REVERSE) {
                pwm_pin_set_cycles(pwm3_dev, MOTOR_1_0_PWM_PORT, 200, 200, 0);  // V BAK
                pwm_pin_set_cycles(pwm3_dev, MOTOR_1_1_PWM_PORT, 200, speed_value, 0);  // V BAK
            }
            break;

        case MOTOR_RIGHT:
            if (dir == DIR_FORWARD) {
                pwm_pin_set_cycles(pwm1_dev, MOTOR_0_0_PWM_PORT, 200, 200, 0);   // H FREM
                pwm_pin_set_cycles(pwm1_dev, MOTOR_0_1_PWM_PORT, 200, speed_value, 0);   // H FREM
            } else if (dir == DIR_REVERSE) {
                pwm_pin_set_cycles(pwm1_dev, MOTOR_0_0_PWM_PORT, 200, speed_value, 0);   // H BAK
                pwm_pin_set_cycles(pwm1_dev, MOTOR_0_1_PWM_PORT, 200, 200, 0);   // H BAK
            }
            break;

        case MOTOR_EXT:
            // For something like the linmot linear motor used in ELE340, another GPIO output would be
            // needed for controlling direction. Example for that can be found in the ELE340 project.
            // For Other motor drivers other sets of frequency / PWM can be set to control the motor.
            LOG_ERR("External motor logic is not defined. Need to first choose a motor.");
            if (dir == DIR_FORWARD) {
                // pwm_pin_set_cycles(pwm0_dev, MOTOR_EXT_PWM_PORT, 200, 200, 0);   // EXT MOTOR
            } else if (dir == DIR_REVERSE) {
                // pwm_pin_set_cycles(pwm0_dev, MOTOR_EXT_PWM_PORT, 200, 200, 0);   // EXT MOTOR
            }
            break;

        default:
            LOG_ERR("Invalid motor id used to set speed!");
    }
    return 0;
}

int motor_servo_set_angle(int angle) {
    // pwm_pin_set_cycles(pwm1_dev, SERVO_PWM_PORT, 200, 100, 0);   // SERVO
    /* all in micro second */
    // STEP 100    /* PWM pulse step */
    // MINPULSEWIDTH 1000  /* Servo 0 degrees */
    // MIDDLEPULSEWIDTH 1500   /* Servo 90 degrees */
    // MAXPULSEWIDTH 2000  /* Servo 180 degrees */
    return 0;
}
