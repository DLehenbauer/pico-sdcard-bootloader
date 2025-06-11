/**
 * https://github.com/DLehenbauer/pico-sdcard-bootloader
 * SPDX-License-Identifier: 0BSD
 */

#pragma once

// Standard
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// SDK
#include <boot/uf2.h>

// Project
#include "prog.h"

void transport_init();  // Initializes the transport layer.
bool uf2_exists();      // Returns true if new firmware is available.

// Reads the firmware file and invokes the callback for each UF2 block.
bool read_uf2(prog_t* prog, accept_block_cb_t callback);

// Removes the UF2 file after reading it.
bool remove_uf2();
