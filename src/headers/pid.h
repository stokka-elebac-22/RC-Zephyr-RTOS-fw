/*
 * Copyright (c) 2021 
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef SRC_HEADERS_PID_H_
#define SRC_HEADERS_PID_H_

#include "../headers/definitions.h"

struct PIDController {
    struct pid_gains pid_gains;   // Controller gains
    struct limits output_limits;         // Output limits
    struct limits integrator_limits;   // Integrator limits

    float tau;    // Derivative low-pass filter time constant
    float T;      // Sample time (in seconds)

    float proportional;            /* Required for graph      */
    float prevError;            /* Required for integrator */
    float integrator;           /* Controller "memory" */
    float prevMeasurement;        /* Required for differentiator */
    float differentiator;

    float out;       // Controller output
};

void  PIDController_Init(struct PIDController *pid);
float PIDController_Update(struct PIDController *pid, float setpoint, float measurement);

#endif  // SRC_HEADERS_PID_H_
