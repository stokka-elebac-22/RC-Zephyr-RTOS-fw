/*
 * Copyright (c) 2021 
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef SRC_HEADERS_SENSOR_READING_H_
#define SRC_HEADERS_SENSOR_READING_H_

int init_sensors();
int make_reading_and_send();
void log_output_all_sensors();
#endif  // SRC_HEADERS_SENSOR_READING_H_
