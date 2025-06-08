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

// Pico SDK
#include <boot/uf2.h>
#include <hardware/flash.h>

// Project
#include "interval_set.h"

#define PROG_AREA_SIZE (PICO_FLASH_SIZE_BYTES - BOOTLOADER_SIZE)
#define PROG_AREA_BEGIN (XIP_BASE)
#define PROG_AREA_END (PROG_AREA_BEGIN + PROG_AREA_SIZE)

#ifdef __cplusplus
extern "C" {
#endif

// Forward declaration of prog_t
struct prog_s;
typedef struct prog_s prog_t;

typedef bool (*accept_block_cb_t)(prog_t* prog, const struct uf2_block* block);

typedef struct prog_s {
    interval_set_t pages_written;           // The set of pages to be written when flashing the UF2 file.
    interval_set_t sectors_erased;          // The set of sectors to be erased before flashing the UF2 file.
    uint32_t num_blocks;                    // Total number of blocks in UF2 file
    uint32_t num_blocks_accepted;           // Number of blocks received so far
    accept_block_cb_t accept_block;         // Invoked for each valid program block received.
    uint8_t vector_table[FLASH_PAGE_SIZE];  // Pending vector table to write at the end of the programming process
    bool has_vector_table;                  // True if the vector table was found in the UF2 file
    bool is_different;                      // True if the UF2 file differs from the current flash contents
} prog_t;

void prog_init(prog_t* prog);
void prog_free(prog_t* prog);

bool process_block(prog_t* prog, const struct uf2_block* block);

#ifdef __cplusplus
}  // extern "C"
#endif
