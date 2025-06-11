/**
 * https://github.com/DLehenbauer/pico-sdcard-bootloader
 * SPDX-License-Identifier: 0BSD
 */

#pragma once

void led_on();
void led_off();
void led_toggle();

typedef enum diag_code_s {
    DIAG_ENTERING_FIRMWARE = 0,
    FATAL_WATCHDOG_WITHOUT_FIRMWARE = 1,
    DIAG_NO_FIRMWARE = 2,
    FATAL_FLASH_FAILED = 3,
    FATAL_INVALID_UF2 = 4,
    DIAG_DELETE_FAILED = 5,
    DIAG_SKIPPED_PROGRAMMING = 6,
} diag_code_t;

void diag_init(void);
void diag(diag_code_t code);
void fatal(diag_code_t code);
