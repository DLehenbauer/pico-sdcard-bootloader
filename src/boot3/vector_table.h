/**
 * https://github.com/DLehenbauer/pico-sdcard-bootloader
 * SPDX-License-Identifier: 0BSD
 */

#pragma once

// Standard
#include <stdint.h>

// Pico SDK
#include <hardware/flash.h>

#define VECTOR_TABLE_ADDR (XIP_BASE + 0x100)
#define VECTOR_TABLE_SIZE 0xC0

extern const volatile uint32_t* vector_table;

#define VECTOR_TABLE_SP_OFFSET 0
#define VECTOR_TABLE_PC_OFFSET 1

bool check_vector_table(const volatile uint32_t* vt);
