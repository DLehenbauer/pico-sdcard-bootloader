/**
 * https://github.com/DLehenbauer/pico-sdcard-bootloader
 * SPDX-License-Identifier: 0BSD
 */

#pragma once

// Standard
#include <stdint.h>
#include <stddef.h>

void flash_erase(uint32_t flash_offs, size_t count);
void flash_prog(uint32_t flash_offs, const uint8_t *data, size_t count);
