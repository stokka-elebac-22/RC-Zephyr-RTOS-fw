/*
 * Copyright (c) 2021 
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "headers/pid.h"

void PIDController_Init(struct PIDController *pid) {
    /* Clear controller variables */
    pid->integrator = 0.0f;
    pid->prevError  = 0.0f;

    pid->differentiator  = 0.0f;
    pid->prevMeasurement = 0.0f;

    pid->out = 0.0f;
}

float PIDController_Update(struct PIDController *pid, float setpoint, float measurement) {
    float error = setpoint - measurement;               // Error signal
    pid->proportional = pid->pid_gains.K_p * error;      // Proportional
    /* Integral    */
    pid->integrator = pid->integrator + 0.5f * pid->pid_gains.K_i * pid->T * (error + pid->prevError);
    /* Anti-wind-up via integrator clamping */
    if (pid->integrator > pid->integrator_limits.max) {
        pid->integrator = pid->integrator_limits.max;
    } else if (pid->integrator < pid->integrator_limits.min) {
        pid->integrator = pid->integrator_limits.min;
    }

    /* Derivative (band-limited differentiator)     
        Note: derivative on measurement, therefore minus sign in front of equation! */
/*    pid->differentiator = -(2.0f * pid->pid_gains.K_d * (measurement - pid->prevMeasurement)
                        + (2.0f * pid->tau - pid->T) * pid->differentiator)
                        / (2.0f * pid->tau + pid->T);
*/
    pid->differentiator = pid->pid_gains.K_d * (measurement - pid->prevMeasurement) / pid->T;
    /* Compute output and apply limits    */
    pid->out = pid->proportional + pid->integrator + pid->differentiator;

    if (pid->out > pid->output_limits.max) {
        pid->out = pid->output_limits.max;
    } else if (pid->out < pid->output_limits.min) {
        pid->out = pid->output_limits.min;
    }

    /* Store error and measurement for later use */
    pid->prevError       = error;
    pid->prevMeasurement = measurement;

    /* Return controller output */
    return pid->out;
}