/* sensor_as7341.h - header file for AS7341 11 channel color sensor driver */

/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef DRIVERS_AS7341_H_AS7341_H_
#define DRIVERS_AS7341_H_AS7341_H_

#include <device.h>
#include <kernel.h>
#include <drivers/sensor.h>

#define AS7341_CONFIG 0x70

// ENABLE Register
#define AS7341_ENABLE 0x80
#define FDEN    1<<7  // Flicker Detection Enable.
#define SMUXEN  1<<4  // SMUX Enable.
#define WEN  1<<1     // Wait Enable.
#define SP_ON  1<<1   // Spectral Measurement Enable.
#define PON 1<<0      // Power ON.

#define SPECRAT_CONFIG 0x00

#define AS7341_REG_CHIP_ID 0x92      ///< Chip ID register
#define AS7341_DEFAULT_CHIP_ID 0x24  ///< Default Chip ID

#define AS7341_CH0_DATA_L 0x95  ///< ADC Channel Data
#define AS7341_CH0_DATA_H 0x96  ///< ADC Channel Data
#define AS7341_CH1_DATA_L 0x97  ///< ADC Channel Data
#define AS7341_CH1_DATA_H 0x98  ///< ADC Channel Data
#define AS7341_CH2_DATA_L 0x99  ///< ADC Channel Data
#define AS7341_CH2_DATA_H 0x9A  ///< ADC Channel Data
#define AS7341_CH3_DATA_L 0x9B  ///< ADC Channel Data
#define AS7341_CH3_DATA_H 0x9C  ///< ADC Channel Data
#define AS7341_CH4_DATA_L 0x9D  ///< ADC Channel Data
#define AS7341_CH4_DATA_H 0x9E  ///< ADC Channel Data
#define AS7341_CH5_DATA_L 0x9F  ///< ADC Channel Data
#define AS7341_CH5_DATA_H 0xA0  ///< ADC Channel Data

#define AS7341_STATUS2 0xA3    ///< Measurement status flags; saturation, validity

/**
 * @brief Sensor channels.
 */
enum as7341_custom_sensor_channel {
    SENSOR_CHAN_F1_415 = SENSOR_CHAN_PRIV_START,
    SENSOR_CHAN_F2_445,
    SENSOR_CHAN_F3_480,
    SENSOR_CHAN_F4_515,
    SENSOR_CHAN_F5_555,
    SENSOR_CHAN_F6_590,
    SENSOR_CHAN_F7_630,
    SENSOR_CHAN_F8_680,
    SENSOR_CHAN_NIR,
    SENSOR_CHAN_CLEAR,
    SENSOR_CHAN_FD
};

struct as7341_driver_data {
    uint16_t channel_f1_415nm_sample;
    uint16_t channel_f2_445nm_sample;
    uint16_t channel_f3_480nm_sample;
    uint16_t channel_f4_515nm_sample;
    uint16_t channel_f5_555nm_sample;
    uint16_t channel_f6_590nm_sample;
    uint16_t channel_f7_630nm_sample;
    uint16_t channel_f8_680nm_sample;
    uint16_t channel_nir;
    uint16_t channel_clear;
    uint16_t channel_fd;
};

/* 
#define AS7341_COMMAND_I_REG        0x00
#define AS7341_OPMODE_SHIFT        5
#define AS7341_OPMODE_MASK        (7 << AS7341_OPMODE_SHIFT)
#define AS7341_INT_BIT_SHIFT        2
#define AS7341_INT_BIT_MASK        (1 << AS7341_INT_BIT_SHIFT)
#define AS7341_INT_PRST_SHIFT        0
#define AS7341_INT_PRST_MASK        (3 << AS7341_INT_BIT_SHIFT)

#define AS7341_OPMODE_OFF        0
#define AS7341_OPMODE_ALS_ONCE    1
#define AS7341_OPMODE_IR_ONCE        2
#define AS7341_OPMODE_ALS_CONT    5
#define AS7341_OPMODE_IR_CONT        6

#define AS7341_COMMAND_II_REG        0x01
#define AS7341_LUX_RANGE_SHIFT    0
#define AS7341_LUX_RANGE_MASK        (3 << AS7341_LUX_RANGE_SHIFT)
#define AS7341_ADC_RES_SHIFT        2
#define AS7341_ADC_RES_MASK        (3 << AS7341_ADC_RES_SHIFT)

#define AS7341_DATA_LSB_REG        0x02
#define AS7341_DATA_MSB_REG        0x03
#define AS7341_INT_LT_LSB_REG        0x04
#define AS7341_INT_LT_MSB_REG        0x05
#define AS7341_INT_HT_LSB_REG        0x06
#define AS7341_INT_HT_MSB_REG        0x07

#define AS7341_ID_REG            0x0F
#define AS7341_BOUT_SHIFT        7
#define AS7341_BOUT_MASK        (1 << AS7341_BOUT_SHIFT)
#define AS7341_ID_SHIFT        3
#define AS7341_ID_MASK        (3 << AS7341_ID_SHIFT)

#if CONFIG_AS7341_MODE_ALS
    #define AS7341_ACTIVE_OPMODE        AS7341_OPMODE_ALS_CONT
    #define AS7341_ACTIVE_CHAN        SENSOR_CHAN_LIGHT
#elif CONFIG_AS7341_MODE_IR
    #define AS7341_ACTIVE_OPMODE        AS7341_OPMODE_IR_CONT
    #define AS7341_ACTIVE_CHAN        SENSOR_CHAN_IR
#endif

#define AS7341_ACTIVE_OPMODE_BITS        \
    (AS7341_ACTIVE_OPMODE << AS7341_OPMODE_SHIFT)

#if CONFIG_AS7341_LUX_RANGE_1K
    #define AS7341_LUX_RANGE_IDX        0
    #define AS7341_LUX_RANGE        1000
#elif CONFIG_AS7341_LUX_RANGE_4K
    #define AS7341_LUX_RANGE_IDX        1
    #define AS7341_LUX_RANGE        4000
#elif CONFIG_AS7341_LUX_RANGE_16K
    #define AS7341_LUX_RANGE_IDX        2
    #define AS7341_LUX_RANGE        16000
#elif CONFIG_AS7341_LUX_RANGE_64K
    #define AS7341_LUX_RANGE_IDX        3
    #define AS7341_LUX_RANGE        64000
#endif

#define AS7341_LUX_RANGE_BITS            \
    (AS7341_LUX_RANGE_IDX << AS7341_LUX_RANGE_SHIFT)

#if CONFIG_AS7341_INTEGRATION_TIME_26
    #define AS7341_ADC_RES_IDX        3
#elif CONFIG_AS7341_INTEGRATION_TIME_410
    #define AS7341_ADC_RES_IDX        2
#elif CONFIG_AS7341_INTEGRATION_TIME_6500
    #define AS7341_ADC_RES_IDX        1
#elif CONFIG_AS7341_INTEGRATION_TIME_105K
    #define AS7341_ADC_RES_IDX        0
#endif

#define AS7341_ADC_RES_BITS            \
    (AS7341_ADC_RES_IDX << AS7341_ADC_RES_SHIFT)

#define AS7341_ADC_DATA_BITS    (16 - 4 * AS7341_ADC_RES_IDX)
#define AS7341_ADC_DATA_MASK    (0xFFFF >> (16 - AS7341_ADC_DATA_BITS))

#if CONFIG_AS7341_INT_PERSIST_1
    #define AS7341_INT_PRST_IDX        0
    #define AS7341_INT_PRST_CYCLES    1
#elif CONFIG_AS7341_INT_PERSIST_4
    #define AS7341_INT_PRST_IDX        1
    #define AS7341_INT_PRST_CYCLES    4
#elif CONFIG_AS7341_INT_PERSIST_8
    #define AS7341_INT_PRST_IDX        2
    #define AS7341_INT_PRST_CYCLES    8
#elif CONFIG_AS7341_INT_PERSIST_16
    #define AS7341_INT_PRST_IDX        3
    #define AS7341_INT_PRST_CYCLES    16
#endif

#define AS7341_INT_PRST_BITS            \
    (AS7341_INT_PRST_IDX << AS7341_INT_PRST_SHIFT)
*/

#endif  // DRIVERS_AS7341_H_AS7341_H_
