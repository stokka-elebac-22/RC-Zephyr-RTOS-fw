// "Copyright 2022 Horti-proj"
#ifndef SRC_HEADERS_HORT_SETTINGS_H_
#define SRC_HEADERS_HORT_SETTINGS_H_
#include <settings/settings.h>

struct led_level settings_get_led_levels(uint8_t index);
struct led_timer settings_get_led_timers(uint8_t index);
int settings_get_lock_mode();
int get_code_if_enabled(int code_id, char *code_buf);
uint32_t settings_get_system_timer_interval(uint16_t channel);

int settings_set_led_level(uint8_t index, uint8_t color, uint8_t value);
int settings_set_led_mode(int16_t channel, int32_t value);
int settings_set_led_level_raw(uint16_t channel, uint16_t value);
int settings_set_led_time_on_raw(int16_t channel, int32_t value);
int settings_set_led_time_off_raw(int16_t channel, int32_t value);
int settings_set_led_dim_time_raw(int16_t channel, int32_t value);
int settings_get_led_mode(int i);

int settings_set_led_time_on(uint8_t hour, uint8_t minute, uint8_t index);
int settings_set_led_time_off(uint8_t hour, uint8_t minute, uint8_t index);
int settings_set_dim_time(uint8_t minute, uint8_t index);

int hort_settings_init(void);
int hort_settings_store(uint32_t new_setting_timestamp);
int settings_parse_and_set(int16_t settings_type, int16_t channel, int32_t value);

uint32_t get_current_setting_timestamp();

uint16_t settings_get_msg_dev_id();
int settings_set_msg_dev_id(uint16_t new_dev_id);
uint32_t settings_get_fwchannel();

enum settings_list {
    DEV_TYPE = 0,
    FW_BRANCH = 1,
    NETWORK_ID = 9,
    DIM_TIME = 10,
    TIME_ON = 11,
    TIME_OFF = 12,
    PWM_VAL = 13,
    LED_MODE = 14,
    LOG_INTERVAL = 20,
    DEFAULT_POS = 30,
    DEFAULT_SPEED = 40,
    
    DOORLOCK_MODE = 50,
    DOORLOCK_OPEN_TIME = 52,
    DOORLOCK_CODE = 53,
    DOORLOCK_CODE_VALID = 54,    
};
// void setI2CLedStatus(uint8_t i2c_index, uint8_t new_status);
// uint8_t getI2CLedStatus(uint8_t i2c_index);


#endif  // SRC_HEADERS_HORT_SETTINGS_H_
