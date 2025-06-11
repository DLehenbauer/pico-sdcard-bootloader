# Pico SD Card Bootloader

A bootloader for the Raspberry Pi Pico (RP2040) that allows updating firmware from a microSD card.

## Overview

This project allows the firmware of the RP2040 microcontroller to be flashed from a microSD card.
It is useful for designs where the USB port is inaccessible, otherwise occupied, or it is inconvenient to require a laptop to update firmware.

### Key Features

- Uses standard .uf2 files
- Does not require a custom linker script
- 2-pass approach validates firmware integrity before flashing

## Quick Start

### 1. Connect a microSD adapter

Connect a 3.3V-compatible microSD adapter to your Raspberry Pi Pico:

| Pico Pin | GPIO      | Adapter Pin | Description               |
|----------|-----------|-------------|---------------------------|
| 21       | 16 (RX)   | DO          | Data out (from SD card)   |
| 22       | 17        | CS / SS     | Chip select               |
| 24       | 18 (SCK)  | SCK         | Serial clock              |
| 25       | 19 (TX)   | DI          | Data in (to SD card)      |
| 23       | GND       | GND / VSS   | Ground                    |
| 36       | 3V3 (Out) | 3V3 / VCC   | Power                     |
| 29       | 22 (In)   | CD or DAT3  | Card detect (if used)     |

### 2. Install Bootloader (One-Time Setup)

1. Connect the Pico via USB while holding the BOOTSEL button
2. Copy [sd_bootloader_v1.0.uf2](dist/sd_bootloader_v1.0.uf2) to the mounted USB drive
3. The Pico will restart and flash "N" in Morse code (long-short pattern) indicating no firmware

### 3. Update firmware via microSD card

1. Copy the example [firmware.uf2](dist/firmware.uf2) to a FAT32-formatted microSD card
2. Insert the card into the adapter connected to your Pico
3. The LED will blink rapidly during flashing, then the device will restart
4. When successful, the LED will "breathe" as it fades in and out

## How It Works

The normal [RP2040 boot process](https://vanhunteradams.com/Pico/Bootloader/Boot_sequence.html) is as follows:

1. The ROM bootloader runs loads the stage 2 bootloader from the first block of flash.
2. The stage 2 bootloader configures the flash for XIP (execute in place) and jumps to the main program.

This project modifies this process by:

1. The ROM bootloader runs the modified stage 2 bootloader
2. A modified stage 2 bootloader jumps to a new stage 3 bootloader that resides in the last 64kB of flash
3. The stage 3 bootloader checks for an inserted microSD card with a 'firmware.uf2' file
4. If found, it validates the UF2 file and writes it to flash.
5. If no update is needed, it runs the existing firmware

Additionally, the bootloader preserves itself during firmware updates by:
2. Skipping the stage 2 bootloader included in the .uf2 file.
3. Validating that the new firmware does not overwrite the last 64kB where the stage 3 bootloader resides.

## Customizing

SPI pins, firmware filename, and diagnostic output are configurable in [config.cmake](config.cmake).

## Related Projects

* [Hachi (八)](https://github.com/muzkr/hachi)
* [Pico SD Bootloader](https://github.com/julienfdev/pico-sd-bootloader)
* [Raspberry Pi SD Card Bootloader](https://github.com/oyama/pico-sdcard-boot)
* [Picocalc_SD_Boot](https://github.com/adwuard/Picocalc_SD_Boot)

## License

This project is licensed under the [0BSD license](https://opensource.org/licenses/0BSD), except where noted in source files or [NOTICE.md](NOTICE.md).
