/*
 * Copyright (c) 2021 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <pico/asm_helper.S>
#include <hardware/regs/m0plus.h>

pico_default_asm_setup

.section .text

.global vector_into_flash

// MODIFIED: This is the original 'vector_into_flash' subroutine extracted from 'exit_from_boot2.S'.
//           This invoked by the stage 3 bootloader to start the main program.

vector_into_flash:
    ldr r0, =(XIP_BASE + 0x100)
    ldr r1, =(PPB_BASE + M0PLUS_VTOR_OFFSET)
    str r0, [r1]
    ldmia r0, {r0, r1}
    msr msp, r0
    bx r1
