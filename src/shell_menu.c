/*
 * Copyright (c) 2021
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifdef CONFIG_SHELL
#include <zephyr.h>
#include <shell/shell.h>
#include <version.h>
#include <logging/log.h>
#include <stdlib.h>
#include <ctype.h>

#include "headers/definitions.h"
// #include "headers/hort_settings.h"
#include "headers/pwm_control.h"
#include "headers/git.h"

LOG_MODULE_REGISTER(app);
// int8_t check_valid_data(const struct shell *shell, uint8_t panel,
//     uint8_t hour, uint8_t minute, uint8_t color, uint8_t level) {
//     if (level > 100) {
//         shell_print(shell, "Invalid LED-level entered");
//         return -1;
//     }
//     if (color > MAX_COLOURS) {
//         shell_print(shell, "Invalid Colour entered");
//         return -1;
//     }
//     if (hour > 24 || minute >= 60) {
//         shell_print(shell, "Invalid time entered");
//         return -1;
//     }
//     if (panel >= MAX_LEDPANELS) {
//         shell_print(shell, "Invalid panel entered");
//         return -1;
//     }
//     return 0;
// }
// static int set_LedPanel(const struct shell *shell, size_t argc, char **argv) {
//     uint8_t current_panel = atoi(argv[1]);
//     uint8_t color = atoi(argv[2]);
//     uint8_t value = atoi(argv[3]);
//     if (check_valid_data(shell, current_panel, 0, 0, color, value) < 0) {
//         return -1;
//     }
//     settings_set_led_level(current_panel, color, value);
//     shell_print(shell, "LedPanel %d - LED %d: %d", current_panel, color, value);
//     return 0;
// }

static int menu_set_motors(const struct shell *shell, char **argv, uint8_t state) {
    shell_print(shell, "Setting motor speed ... %s", argv[0]);

    uint8_t motor = atoi(argv[1]);
    uint8_t dir = atoi(argv[2]);
    uint8_t speed = atoi(argv[3]);

    // uint8_t speed_b = 0; // atoi(argv[4]);

    shell_print(shell, "m%d: %d - %d",
        motor, dir, speed);

    if (speed > 201) { speed = 201; }
    switch (motor) {
        case 0:
            if (dir) {
                motor_set_speed(MOTOR_LEFT, DIR_FORWARD, speed);
            } else {
                motor_set_speed(MOTOR_LEFT, DIR_REVERSE, speed);
            }
            break;
        case 1:
            if (dir) {
                motor_set_speed(MOTOR_RIGHT, DIR_FORWARD, speed);
            } else {
                motor_set_speed(MOTOR_RIGHT, DIR_REVERSE, speed);
            }
            break;
    }
    shell_print(shell, "m%d: %d - %d",
        motor, dir, speed);
    return 0;
}

char* is_dirty() {
    if (GIT_IS_DIRTY) {
        return "-DIRTY";
    } else {
        return "";
    }
}

static int cmd_fw_version(const struct shell *shell) {
    shell_print(shell, "Zephyr version %s", KERNEL_VERSION_STRING);
    shell_print(shell, "RC-Controller firmware version %s-%s%s", GIT_BRANCH, GIT_DESCRIBE, is_dirty());
    return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(sub_rc,
    // SHELL_CMD_ARG(led-intensity, NULL, " led-intensity [ledpanel] [color] [intensity 0-255]", set_LedPanel, 3, 1),
    // SHELL_CMD_ARG(led-start, NULL, "led-start [ledpanel] [HH] [MM]", set_LedPanel_on, 3, 1),
    // SHELL_CMD_ARG(led-stop, NULL, "led-stop [ledpanel] [HH] [MM]", set_LedPanel_off, 3, 1),
    // SHELL_CMD_ARG(led-state, NULL, "led-state [ledpanel] [ACTIVE (1) / INACTIVE (0)]", set_LedPanel_mode, 2, 1),
    SHELL_CMD_ARG(motor-set-speed, NULL, "motor-set-speed [dir a] [v a] [dir b] [v b]", menu_set_motors, 4, 0),
    SHELL_CMD(sysver, NULL, "Show System version", cmd_fw_version),
    SHELL_SUBCMD_SET_END /* Array terminated. */
);

SHELL_CMD_REGISTER(rc_controller, &sub_rc, "RC-Controller Commands", NULL);
#endif  // CONFIG_SHELL
