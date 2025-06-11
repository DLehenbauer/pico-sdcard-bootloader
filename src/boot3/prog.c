/**
 * https://github.com/DLehenbauer/pico-sdcard-bootloader
 * SPDX-License-Identifier: 0BSD
 */

// Standrad
#include <string.h>

// Pico SDK
#include <pico/assert.h>

// Project
#include "prog.h"
#include "vector_table.h"

#if NDEBUG
#define assert_flash_address(addr) ((void)0)
#else
void assert_flash_address(uint32_t addr) {
    assert(XIP_BASE <= addr && addr < SRAM_BASE);
}
#endif

uint32_t page_index(uint32_t addr) {
    assert_flash_address(addr);
    return (addr - XIP_BASE) / FLASH_PAGE_SIZE;
}

uint32_t sector_index(uint32_t addr) {
    assert_flash_address(addr);
    return (addr - XIP_BASE) / FLASH_SECTOR_SIZE;
}

void prog_init(prog_t* prog) {
    memset(prog, 0, sizeof(prog_t));
    interval_set_init(&prog->pages_written);
    interval_set_init(&prog->sectors_erased);
}

void prog_free(prog_t* prog) {
    interval_set_free(&prog->pages_written);
    interval_set_free(&prog->sectors_erased);
    memset(prog, 0, sizeof(prog_t));
}

// Called by the transport for each UF2 block.
bool process_block(prog_t* prog, const struct uf2_block* block) {
    // Must be a valid UF2 block.
    bool ok = ((block->magic_start0 == UF2_MAGIC_START0) &&     // Block must start with magic numbers 
               (block->magic_start1 == UF2_MAGIC_START1) && 
               (block->magic_end == UF2_MAGIC_END));            // Block must end with magic number

    // If the family Id is missing or does not match the expected RP2040 family ID, then skip the
    // block.  The UF2 specification allows programs for multiple targets to be concatenated in a
    // single UF2 file.  The family ID is used to select the correct program for the target.
    //
    // According to the UF2 specification, the UF2_FLAG_FAMILY_ID_PRESENT is optional, but it is
    // recommended that bootloaders require it.  The RP2040 bootrom skips blocks that do not have
    // the family ID present, so we do the same here.
    //
    // Note that when the UF2_FLAG_FAMILY_ID_PRESENT flag is set, the 'file_size' field contains the
    // family ID.
    if (((block->flags & UF2_FLAG_FAMILY_ID_PRESENT) == 0 || (block->file_size != RP2040_FAMILY_ID))) {
        // If block is part of a program for another target (but is otherwise a valid UF2 block),
        // ignore it and continue programming.
        return ok;
    }

    // Each block has a 'num_blocks' field that indicates the total number of blocks in the program.
    // We capture this value from the first block we process and require that subsequent blocks
    // report the same number of total blocks.
    if (prog->num_blocks_accepted == 0) {
        // 'num_blocks' must be greater than 0, since the program has at least one block
        // (i.e., the block we're processing now).
        ok &= (block->num_blocks > 0);

        // Record the number of blocks in the program for validating subsequent blocks.
        prog->num_blocks = block->num_blocks;
    } else  {
        // All blocks in the program must report the same total number of blocks.
        ok &= (block->num_blocks == prog->num_blocks);
    }

    // The 'block_no' field indicates the sequential index of this block within the program.
    // It must start at 0 and increment by 1 for each subsequent block.
    ok &= block->block_no == prog->num_blocks_accepted;

    // The 'block_no' field must be less than the total number of blocks in the program.
    ok &= block->block_no < prog->num_blocks;

    // The UF2 spec allows metadata blocks that are not intended to be written to flash.
    // These are indicated by the UF2_FLAG_NOT_MAIN_FLASH flag and should be skipped.
    if ((block->flags & UF2_FLAG_NOT_MAIN_FLASH) != 0) {
        // If this block is not for the main flash (but is otherwise valid), ignore it
        // and continue programming.
        return ok;
    }

    // The target address is the address in flash where the block should be written.
    // In the RP2040 memory map, flash memory begins at XIP_BASE.
    uint32_t start_addr = block->target_addr;
    uint32_t end_addr   = start_addr + block->payload_size;

    // The target address must be aligned to a flash page or 'flash_range_program' will fail.
    ok &= ((start_addr % FLASH_PAGE_SIZE) == 0);

    // Our bootloader requires the block payload size to be exactly one flash page size.
    // (The RP2040 bootrom has the same requirement.)
    ok &= (end_addr - start_addr) == FLASH_PAGE_SIZE;

    // The target address must be within the available program area.
    ok &= (PROG_AREA_BEGIN <= start_addr) && (end_addr <= PROG_AREA_END);

    if (block->target_addr == VECTOR_TABLE_ADDR) {
        // Note that a valid vector table was found.
        prog->has_vector_table = check_vector_table((const volatile uint32_t*) block->data);

        // Abort progarmming if vector table was invalid.
        ok &= prog->has_vector_table;
    }

    // If we failed any of the checks above, exit early before we update the interval sets.
    if (!ok) { return false; }

    // The flash page must not have been previously been written.
    ok &= (interval_set_union(&prog->pages_written, page_index(start_addr), page_index(end_addr)) == 1);

    // While flash pages are 256 bytes, the smallest unit we can erase is a flash sector, which is 4kB.
    // Add the sectors intersected by the current block to the set of sectors that need to be erased.
    uint32_t start_sector = sector_index(start_addr);
    uint32_t end_sector   = MAX(sector_index(end_addr), start_sector + 1);
    interval_set_union(&prog->sectors_erased, start_sector, end_sector);

    // If an overlapping write was detected, exit early before accepting the block.
    if (!ok) { return false; }

    ok &= prog->accept_block(prog, block);

    if (ok) {
        prog->num_blocks_accepted++;
    }

    return ok;
}
