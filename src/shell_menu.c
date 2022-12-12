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

// #include "headers/definitions.h"
// #include "headers/hort_settings.h"
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

// static int set_LedPanelTime(const struct shell *shell, char **argv, uint8_t state) {
//     uint8_t current_panel = atoi(argv[1]);
//     uint8_t newHour = atoi(argv[2]);
//     uint8_t newMinute = atoi(argv[3]);
//     if (newHour == 24) { newHour = 0; }
//     if (check_valid_data(shell, current_panel, newHour, newHour, 0, 0) < 0) {
//         return -1;
//     }
//     if (state == 1) {
//         settings_set_led_time_on(newHour, newMinute, current_panel);
//     } else {
//         settings_set_led_time_off(newHour, newMinute, current_panel);
//     }
//     shell_print(shell, "LedPanel %s - LED %d: %d:%d", (state == 1) ? "ON" : "OFF", current_panel, newHour, newMinute);
//     return 0;
// }
// static int set_LedPanel_on(const struct shell *shell, size_t argc, char **argv) {
//     return set_LedPanelTime(shell, argv, 1);
// }

// static int set_LedPanel_off(const struct shell *shell, size_t argc, char **argv) {
//     return set_LedPanelTime(shell, argv, 0);
// }

// static int set_LedPanel_mode(const struct shell *shell, size_t argc, char **argv) {
//     settings_set_led_mode(atoi(argv[1]), atoi(argv[2]));
//     shell_print(shell, "LedPanel %s - Status: %d", argv[1], settings_get_led_mode(atoi(argv[1])));
//     return 0;
// }

// static int get_LedPanel_mode(const struct shell *shell, size_t argc, char **argv) {
//     for (uint8_t i = 0; i < 4; i++) {
//         shell_print(shell, "LedPanel %i - Status: %d", i, settings_get_led_mode(i));
//         struct led_level led_values = settings_get_led_levels(i);
//         shell_print(shell, "  P0: %d   RB: %d  Li: %d  P1: %d",
//             led_values.pink0, led_values.rblue, led_values.lime, led_values.pink1);
//         struct led_timer led_timers = settings_get_led_timers(i);
//         uint8_t hours_on = led_timers.led_time_on/3600;
//         uint8_t hours_off = led_timers.led_time_off/3600;
//         shell_print(shell, "  ON: %d:%d   OFF: %d:%d  Dim-time: %d min Mode: %d \n",
//             hours_on, ((led_timers.led_time_on-hours_on*3600)/60),
//             hours_off, ((led_timers.led_time_off-hours_off*3600)/60),
//             led_timers.led_dim_time, led_timers.mode);
//     }
//     return 0;
// }

char* is_dirty() {
    if (GIT_IS_DIRTY) {
        return "-DIRTY";
    } else {
        return "";
    }
}

static int cmd_fw_version(const struct shell *shell) {
    shell_print(shell, "Zephyr version %s", KERNEL_VERSION_STRING);
    shell_print(shell, "Horticulture LED firmware version %s-%s%s", GIT_BRANCH, GIT_DESCRIBE, is_dirty());
    return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(sub_horti,
    // SHELL_CMD_ARG(led-intensity, NULL, " led-intensity [ledpanel] [color] [intensity 0-255]", set_LedPanel, 3, 1),
    // SHELL_CMD_ARG(led-start, NULL, "led-start [ledpanel] [HH] [MM]", set_LedPanel_on, 3, 1),
    // SHELL_CMD_ARG(led-stop, NULL, "led-stop [ledpanel] [HH] [MM]", set_LedPanel_off, 3, 1),
    // SHELL_CMD_ARG(led-state, NULL, "led-state [ledpanel] [ACTIVE (1) / INACTIVE (0)]", set_LedPanel_mode, 2, 1),
    // SHELL_CMD_ARG(led-print-settings, NULL, "led-print-settings", get_LedPanel_mode, 0, 1),
    SHELL_CMD(sysver, NULL, "Show System version", cmd_fw_version),
    SHELL_SUBCMD_SET_END /* Array terminated. */
);

SHELL_CMD_REGISTER(horti, &sub_horti, "Horticulture Commands", NULL);
#endif  // CONFIG_SHELL
