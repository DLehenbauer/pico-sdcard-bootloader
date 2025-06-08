/**
 * BSD Zero Clause License (SPDX: 0BSD)
 * 
 * Permission to use, copy, modify, and/or distribute this software for any purpose
 * with or without fee is hereby granted.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
 * THIS SOFTWARE.
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
