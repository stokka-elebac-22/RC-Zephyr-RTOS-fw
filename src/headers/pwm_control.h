/*
 * Copyright (c) 2021 
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef SRC_HEADERS_PWM_CONTROL_H_
#define SRC_HEADERS_PWM_CONTROL_H_
#include "../headers/definitions.h"

int motor_init();
int motor_set_speed(enum motor_ids motor, int dir, int speed);
int motor_servo_set_angle(int angle);
#endif  // SRC_HEADERS_PWM_CONTROL_H_
