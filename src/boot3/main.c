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

// Standard
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Pico SDK
#include <boot/uf2.h>
#include <hardware/flash.h>
#include <hardware/watchdog.h>

// Project
#include "diag.h"
#include "flash.h"
#include "prog.h"
#include "transport.h"
#include "vector_table.h"

// During pass 1 (validation), this callback is invoked for each block in the UF2
// file that is valid and matches the expected family ID.
bool validate_uf2_callback(prog_t* prog, const struct uf2_block* block) {
    // Blink the LED to show progress during large files.
    if (prog->num_blocks_accepted % 128 == 0) {
        led_toggle();
    }

    // While validating, we also check if the current contents of flash are identical
    // to the contents of the UF2 file (ignoring the stage2 bootloader).
    if (block->target_addr != XIP_BASE) {
        // Once we've found the first different block, we can stop reading/comparing.
        prog->is_different = prog->is_different
            || memcmp((void*) block->target_addr, block->data, FLASH_PAGE_SIZE) != 0;
    }

    // And continue processing blocks.
    return true;
}

// During pass 2 (writing), this callback is invoked for each block in the UF2
// file that is valid and matches the expected family ID.
bool write_uf2_callback(prog_t* prog, const struct uf2_block* block) {
    // Blink the LED rapidly to show progress during large files.
    if (prog->num_blocks_accepted % 16 == 0) {
        led_toggle();
    }

    // There are a few target addresses that we require special handling.
    switch (block->target_addr) {
        case XIP_BASE:
            // Ignore the stage2 bootloader block from the UF2 file.  We want to preserve our
            // custom boot stage 2 (which we've already restored after erasing the sectors).
            break;

        case VECTOR_TABLE_ADDR:
            // We detect if firmware has been installed by checking for a valid vector table.
            // Delay writing the vector table until the end of the programming process.
            memcpy(prog->vector_table, block->data, FLASH_PAGE_SIZE);
            break;

        default:
            // Normal block: write to flash.
            flash_prog(block->target_addr - XIP_BASE, block->data, FLASH_PAGE_SIZE);
            break;
    }

    // And continue processing blocks.
    return true;
}

void update_firmware() {
    prog_t prog;
    prog_init(&prog);
    prog.accept_block = validate_uf2_callback;

    //
    // Pass 1: Validate the UF2 file
    //

    bool ok = read_uf2(&prog, process_block);

    // Ensure that the entire program was received.
    ok &= (prog.num_blocks > 0);
    ok &= (prog.num_blocks_accepted == prog.num_blocks);

    // Ensure that the program contains a valid vector table.
    ok &= (prog.has_vector_table);

    if (!ok) {
        fatal(FATAL_INVALID_UF2);
    }

    if (!prog.is_different) {
        diag(DIAG_SKIPPED_PROGRAMMING);
        goto done;
    }

    //
    // Pass 2: Write the UF2 file to flash
    //

    // Because there is a valid vector table in the UF2 file, we can assume
    // that sector zero will be erased.
    assert(prog.sectors_erased.num_intervals > 0);
    assert(prog.sectors_erased.intervals[0].start == 0);

    // Backup stage 2 bootloader.
    uint8_t boot2_backup[FLASH_PAGE_SIZE];
    memcpy(boot2_backup, (void*) XIP_BASE, FLASH_PAGE_SIZE);

    // Erase sectors written by the UF2 file.
    led_on();

    for (int i = 0; i < prog.sectors_erased.num_intervals; i++) {
        interval_t* current = &prog.sectors_erased.intervals[i];
        uint32_t sector_start = current->start * FLASH_SECTOR_SIZE;
        uint32_t sector_end = current->end * FLASH_SECTOR_SIZE;
        flash_erase(sector_start, sector_end - sector_start);
    }

    // To improve the odds of recovery in case programming is interrupted, we
    // restore our custom stage 2 bootloader before first.
    flash_prog(0, boot2_backup, FLASH_PAGE_SIZE);

    // Reset our programming state and prepare for writing.
    prog.accept_block = write_uf2_callback;
    prog.num_blocks = 0;
    prog.num_blocks_accepted = 0;
    interval_set_clear(&prog.pages_written);

    ok &= read_uf2(&prog, process_block);
    if (!ok) {
        fatal(FATAL_FLASH_FAILED);
    }

    // Programming is successful.  The only thing left to do is to write the vector table
    // to flash.
    flash_prog(VECTOR_TABLE_ADDR - XIP_BASE, prog.vector_table, FLASH_PAGE_SIZE);

done:
    // Finally, remove the UF2 file to prevent reprogramming on next boot.
    if (ok && !remove_uf2()) {
        diag(DIAG_DELETE_FAILED);
    }

    prog_free(&prog);
    led_off();
}

void run_firmware() {
    // We use the watchdog to reset the cores and peripherals to get back to
    // a known state before running the firmware.  The 'main()' function detects
    // if we are entering from the watchdog and jumps to the vector table.
    watchdog_enable(/* delay_ms: */ 0, /* pause_on_debug: */ true);

    // Wait for the watchdog to kick in and reset the device.
    while (true) {
        busy_wait_at_least_cycles(0xffffffff);
    }
}

int main() {
    diag_init();

    // To ensure that all cores and peripherals are in their initial state, our
    // stage 3 bootloader uses the watchdog to reset the device when it is ready
    // to run the firmware.
    if (watchdog_enable_caused_reboot()) {
        // Clear the watchdog scratch register to avoid mistaking a normal reset
        // for the watchdog.
        watchdog_hw->scratch[4] = 0;

        // Double check that we have a valid vector table before jumping.
        if (!check_vector_table(vector_table)) {
            fatal(FATAL_WATCHDOG_WITHOUT_FIRMWARE);
        }

        // Run the firmware.
        extern void vector_into_flash(uint32_t);
        vector_into_flash(VECTOR_TABLE_ADDR);
    }

    transport_init();

    // Poll for either a new firmware file or a valid vector table.
    while (true) {
        if (uf2_exists()) {
            update_firmware();
        }

        if (check_vector_table(vector_table)) {
            run_firmware();
        }

        diag(DIAG_NO_FIRMWARE);
    }

    assert(false);

    return 0;
}
