// "Copyright 2022 Horti-proj"
#include <zephyr.h>
#include <device.h>
#include <logging/log.h>
#include "headers/definitions.h"

atomic_t status = ATOMIC_INIT(0);

int status_set(int32_t status_code) {
    atomic_set(&status, status_code);
    return 0;
}

int32_t status_get() {
    return (int32_t)atomic_get(&status);
}
