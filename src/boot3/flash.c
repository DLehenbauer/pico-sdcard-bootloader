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
