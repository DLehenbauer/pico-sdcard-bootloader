/**
 * https://github.com/DLehenbauer/pico-sdcard-bootloader
 * SPDX-License-Identifier: 0BSD
 */

#include "vector_table.h"

// Points to the start of the main program's vector table in flash memory (0x10000100).
const volatile uint32_t* vector_table = (const volatile uint32_t *)(uintptr_t) VECTOR_TABLE_ADDR;

// This function returns true if the stack pointer (sp) and program counter (pc)
// in the vector table are within the SRAM and flash ranges, respectively.
bool check_vector_table(const volatile uint32_t* vt) {
    const uint32_t sp = vt[VECTOR_TABLE_SP_OFFSET];
    const uint32_t pc = vt[VECTOR_TABLE_PC_OFFSET] & ~1;         // bit 0 indicates ARM/Thumb encoding
    const bool thumb = (vt[VECTOR_TABLE_PC_OFFSET] & 1) == 1;    // 'true' if Thumb

    // The initial stack pointer (sp) must be within the SRAM range.
    // Note that when the stack is empty, sp == SRAM_END (not SRAM_END - 1).
    bool ok = SRAM_BASE <= sp && sp <= SRAM_END;

    // The RP2040 (Cortex-M0+) only supports the Thumb instruction set.
    ok &= thumb;

    // Thumb instructions are aligned to 2 bytes.
    ok &= (pc % 2 == 0);

    // The stage 2 bootloader & vector table occupy the first 0x1C0 bytes of flash.
    const uint32_t pc_min = VECTOR_TABLE_ADDR + VECTOR_TABLE_SIZE;

    // Our stage 3 bootloader occupies the top 'BOOTLOADER_SIZE' bytes of flash.
    const uint32_t pc_max = XIP_BASE + PICO_FLASH_SIZE_BYTES - BOOTLOADER_SIZE;
    
    // The entry point of the program (pc) must be between.
    ok &= (pc_min <= pc) && (pc < pc_max);
    
    return ok;
}
