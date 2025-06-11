/**
 * https://github.com/DLehenbauer/pico-sdcard-bootloader
 * SPDX-License-Identifier: 0BSD
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
    interval_set_t pages_written;           // Tracks which flash pages have been written to detect overlapping writes.
    interval_set_t sectors_erased;          // Tracks which flash sectors have been written for bulk erasure.
    uint32_t num_blocks;                    // Total number of blocks declared in the UF2 file
    uint32_t num_blocks_accepted;           // Number of blocks accepted for writing so far.
    accept_block_cb_t accept_block;         // Invoked for each valid program block that is accepted for writing.
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
