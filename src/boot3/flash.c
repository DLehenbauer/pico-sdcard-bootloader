/**
 * https://github.com/DLehenbauer/pico-sdcard-bootloader
 * SPDX-License-Identifier: 0BSD
 */

// Pico SDK
#include <hardware/flash.h>
#include <hardware/sync.h>

// Project
#include "flash.h"

void flash_erase(uint32_t flash_offs, size_t count) {
    uint32_t interrupts = save_and_disable_interrupts();
    flash_range_erase(flash_offs, count);
    restore_interrupts(interrupts);
}

void flash_prog(uint32_t flash_offs, const uint8_t *data, size_t count) {
    uint32_t interrupts = save_and_disable_interrupts();
    flash_range_program(flash_offs, data, count);
    restore_interrupts(interrupts);
}
