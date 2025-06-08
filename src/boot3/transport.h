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
