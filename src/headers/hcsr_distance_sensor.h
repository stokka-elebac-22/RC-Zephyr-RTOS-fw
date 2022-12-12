/*
 * Copyright (c) 2021 
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef SRC_HCSR_DISTANCE_SENSOR_H_
#define SRC_HCSR_DISTANCE_SENSOR_H_

void hcsr_get(int sensor_no, int channel, struct sensor_value *sv);
void send_trig();
void hcsr_init();

#endif  // SRC_HCSR_DISTANCE_SENSOR_H_
