// // "Copyright 2022 Horti-proj"
// #include <zephyr.h>
// #include <device.h>
// #include <logging/log.h>
// #include <settings/settings.h>
// #include <inttypes.h>
// #include <errno.h>
// #include <stdio.h>
// #include <stdlib.h>

// #include "headers/definitions.h"
// #include "headers/hort_settings.h"
// #include "headers/otafw.h"

// #define DEFAULT_LOCK_MODE LOCK_NORMAL
// #define DEFAULT_TIME_ON 21600   // gmt 6:00
// #define DEFAULT_TIME_OFF 72000  // gmt 20:00
// #define DEFAULT_DIM_TIME 5   // 5 seconds

// #ifdef CONFIG_OPENTHREAD_MTD_SED
// #define DEFAULT_SYSTEM_TIMERS_TIME 300
// #else
// #define DEFAULT_SYSTEM_TIMERS_TIME 60
// #endif

// LOG_MODULE_REGISTER(hort_settings);

// static int horti_settings_set(const char *name, size_t len, settings_read_cb read_cb, void *cb_arg);

// struct settings_handler my_conf = {
//     .name = "horti",
//     .h_set = horti_settings_set
// };

// struct lock_config_struct running_config;
// // struct lock_config_struct lock_device_config; // mirror "known" device config to see if log of changes has been updated / sent

// int check_valid_code_channel(int16_t channel) {
//     if (channel >= MAX_LOCK_CODES) {
//         LOG_ERR("Channel array out of bounds at %u! \n", channel);
//         return -1;
//     }
//     return 0;
// }

// int settings_lock_update_code(int16_t index, int32_t code) {
//     if (check_valid_code_channel(index) < 0) return -1;
//     running_config.lock_codes[index].code = code;
//     if (code != 0) send_lock_code_updated_log(index);
//     return 0;
// }

// int settings_lock_update_code_valid_time(int16_t index, int32_t valid_time) {
//     // implement code to set valid time
//     return 0;
// }

// int settings_lock_check_code_valid(int16_t index) {
//     // implement code to check if code is still valid (time)
//     // returns -1 if not valid
//     return 0;
// }

// int check_valid_time(int32_t value) {
//     if (value > 86400) {
//         LOG_ERR("LOCK-timer: %u seconds is more than 24 hours \n", value);
//         return -1;
//     } else if (value < 0) {
//         LOG_ERR("LOCK-timer: cannot be less than zero: %u not possible \n", value);
//         return -1;
//     }
//     return 0;
// }

// uint16_t settings_get_msg_dev_id() {
//     return 0;
// }
// int settings_get_led_mode(int i) {
//     return 0;
// }

// int get_code_if_enabled(int code_id, char *code_buf) {
//     if (check_valid_code_channel(code_id) < 0) return -1;
//     if (settings_lock_check_code_valid(code_id) < 0) return -1;
//     if (running_config.lock_codes[code_id].code < LOCK_EN_SIZE) return -1;

// //    int code_id = 13215;
//     char size[3];
//     char snum[20];
//     memset(snum, 0, sizeof snum);
//     sprintf(snum, "%dZ", running_config.lock_codes[code_id].code);
//     memset(size, 0, sizeof size);
//     memcpy(size, &snum[1], 1);
//     int code_size = atoi(size);
//     // char code[code_size + 1];
//     // memset(code, 0, sizeof(code));
//     memcpy(code_buf, &snum[2], code_size);
//     return code_size;
// }

// struct led_timer settings_get_lock_settings() {
//     return running_config.lock_settings;
// }

// int settings_get_lock_mode() {
//     return running_config.lock_settings.mode;
// }

// uint32_t settings_get_system_timer_interval(uint16_t channel) {
//     uint32_t ret_time = 0;
//     if (channel < MAX_SYSTEM_TIMERS) {
//         ret_time = running_config.system_timers[channel];
//         if (!ret_time) ret_time = 15;  // Default 15Secs
//     }
//     return ret_time;
// }
// int settings_set_system_timer_interval(uint16_t channel, uint32_t timer) {
//     if (channel < MAX_SYSTEM_TIMERS) {
//         if (timer)
//             running_config.system_timers[channel] = timer;
//         else
//             return -1;
//     }
//     return 0;
// }

// int settings_set_lock_mode(int32_t value) {
//     if (value >= LOCK_STATES_MAX) {
//         LOG_ERR("Invalid mode: %u \n", value);
//         return -1;
//     } else {
//         running_config.lock_settings.mode = value;
//         send_mode_to_lock(value);
//         return 0;
//     }
// }


// int settings_set_lock_time_on_raw(int32_t value) {
//     running_config.lock_settings.led_time_on = value;
//     return 0;
// }

// int settings_set_lock_time_off_raw(int32_t value) {
//     running_config.lock_settings.led_time_off = value;
//     return 0;
// }

// int settings_set_lock_open_time_raw(int32_t value) {
//     running_config.lock_settings.led_dim_time = value;
//     return 0;
// }

// static int horti_settings_set(const char *name, size_t len, settings_read_cb read_cb, void *cb_arg) {
//     const char *next;
//     if (settings_name_steq(name, "running_config", &next) && !next) {
//         read_cb(cb_arg, &running_config, sizeof(running_config));
//     }
//     return -ENOENT;
// }

// int settings_parse_and_set(int16_t settings_type, int16_t channel, int32_t value) {
//     switch ((int) settings_type) {
//         // Device settings
//         case DEV_TYPE:
//             break;
//         case FW_BRANCH:
//             if (!channel) {
//                 running_config.fw_channel = value;
//                 coap_get_fw();
//             }
//             break;
//         // LOCK settings
//         case DIM_TIME:
//             return -1;  // invalid setting
//         case TIME_ON:
//             return settings_set_lock_time_on_raw(value);
//         case TIME_OFF:
//             return settings_set_lock_time_off_raw(value);
//         case PWM_VAL:
//             return -1;  // invalid setting
//         case LED_MODE:
//             return -1;  // invalid setting
//         case DOORLOCK_MODE:
//             return settings_set_lock_mode(value);
//         case DOORLOCK_OPEN_TIME:
//             return settings_set_lock_open_time_raw(value);
//         case DOORLOCK_CODE:
//             return settings_lock_update_code(channel, value);
//         case DOORLOCK_CODE_VALID:    
//             return settings_lock_update_code_valid_time(channel, value);

//         // Log-settings
//         case LOG_INTERVAL:
//             settings_set_system_timer_interval(SENSORREADING, value);
//             break;
//         default:
//             break;
//     }
//     return 0;
// }

// int hort_settings_init(void) {
//     settings_subsys_init();
//     settings_register(&my_conf);
//     settings_load();

//     // if not a valid config loaded, configure "default settings"
//     if (!(running_config.setting_version.major == SETTINGS_MAJ_VER)) {
//         running_config.setting_version.major = SETTINGS_MAJ_VER;
//         for (int code_id = 0; code_id < MAX_LOCK_CODES; code_id++) {
//             settings_lock_update_code(code_id, 0);
//             settings_lock_update_code_valid_time(code_id, 0);
//         }
//         settings_set_lock_mode(DEFAULT_LOCK_MODE);
//         settings_set_lock_time_on_raw(DEFAULT_TIME_ON);
//         settings_set_lock_time_off_raw(DEFAULT_TIME_OFF);
//         settings_set_lock_open_time_raw(DEFAULT_DIM_TIME);             
        
//         running_config.fw_channel = OTAFW_BRANCH;
//         running_config.setting_timestamp = 0;
//         for (int i = 0; i < MAX_SYSTEM_TIMERS; i++) {
//             running_config.system_timers[i] = DEFAULT_SYSTEM_TIMERS_TIME;
//             #ifdef CONFIG_ARCH_POSIX
//             running_config.system_timers[i] = 10;
//             #endif
//         }
//     }
//     if (!(running_config.setting_version.minor == SETTINGS_MIN_VER)) {
//         running_config.setting_version.minor = SETTINGS_MIN_VER;
//         running_config.setting_timestamp = 0;
//     }
//     return 0;
// }

// int hort_settings_store(uint32_t new_setting_timestamp) {
//     running_config.setting_timestamp = new_setting_timestamp;
//     settings_save_one("horti/running_config", &running_config, sizeof(running_config));
//     return 0;
// }

// uint32_t get_current_setting_timestamp() {
//     return running_config.setting_timestamp;
// }

// uint32_t settings_get_fwchannel() {
//     uint32_t channel = (OTAFW_PROJ << 16 | OTAFW_BOARD << 8 | running_config.fw_channel);
//     return channel;
// }
