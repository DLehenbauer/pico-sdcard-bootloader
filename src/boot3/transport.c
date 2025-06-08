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
#include <stdio.h>
#include <string.h>

// Pico SDK
#include <pico/stdlib.h>

// SPI/FatFS
#include <ff.h>
#include <diskio.h>
#include <f_util.h>
#include <hw_config.h>
#include <rtc.h>

// Project
#include "diag.h"
#include "transport.h"

#define PC_NAME "0:"
#define FIRMWARE_FILENAME (PC_NAME BOOTLOADER_FIRMWARE_FILENAME)

static spi_t spis[] = {{
    .hw_inst    = BOOTLOADER_SD_SPI_INSTANCE,
    .miso_gpio  = BOOTLOADER_SD_SPI_RX_PIN,
    .mosi_gpio  = BOOTLOADER_SD_SPI_TX_PIN,
    .sck_gpio   = BOOTLOADER_SD_SPI_SCK_PIN,
    .baud_rate  = BOOTLOADER_SD_BAUD_RATE
}};

// ext/no-OS-FatFS-SD-SPI-RPi-Pico/FatFs_SPI/sd_driver/hw_config.h

static sd_card_t sd_cards[] = {{
    .pcName = PC_NAME,
    .spi = &spis[0],
    .ss_gpio = BOOTLOADER_SD_SPI_CSN_PIN,
    .card_detect_gpio = BOOTLOADER_SD_DETECT_PIN,
    .use_card_detect = BOOTLOADER_SD_USE_DETECT
}};

size_t sd_get_num() { return count_of(sd_cards); }

sd_card_t* sd_get_by_num(size_t num) {
    return num < sd_get_num()
        ?  &sd_cards[num]
        : NULL;
}

size_t spi_get_num() { return count_of(spis); }

spi_t* spi_get_by_num(size_t num) {
    return num < spi_get_num()
        ? &spis[num]
        : NULL;
}

static FIL file = { 0 };

void transport_init() {
    time_init();
}

bool uf2_exists() {
    sd_card_t* pSd = sd_get_by_num(0);

    if (pSd->mounted && !pSd->sd_test_com(pSd)) {
        f_unmount(pSd->pcName);
        pSd->mounted = false;
    }

    FRESULT fr = FR_OK;

    if (!pSd->mounted) {
        led_on();
        fr = f_mount(&pSd->fatfs, pSd->pcName, 1);
        led_off();
        
        if (fr != FR_OK) {
            pSd->m_Status |= STA_NOINIT;
            pSd->mounted = false;
            return false;
        }
        
        pSd->mounted = true;
    }

    FILINFO fileInfo;
    fr = f_stat(FIRMWARE_FILENAME, &fileInfo);
    return (FR_OK == fr && fileInfo.fsize > 0);
}

bool read_uf2(prog_t* prog, accept_block_cb_t callback) {
    if (!uf2_exists()) {
        return false;
    }

    if (f_open(&file, FIRMWARE_FILENAME, FA_READ | FA_OPEN_EXISTING) != FR_OK) {
        return false;
    }

    while (true) {
        struct uf2_block block;
        size_t bytes_read = 0;

        if (f_read(&file, &block, sizeof(block), &bytes_read) != FR_OK) { goto error; }

        if (bytes_read < sizeof(block)) {
            if (bytes_read == 0) { goto done; }
            else { goto error; }
        }

        if (!callback(prog, &block)) { goto error; }
    }

done:
    f_close(&file);
    f_unlink(FIRMWARE_FILENAME);
    return true;

error:
    f_close(&file);
    return false;
}

bool remove_uf2() {
    FRESULT fr = f_unlink(FIRMWARE_FILENAME);
    return fr == FR_OK;
}
