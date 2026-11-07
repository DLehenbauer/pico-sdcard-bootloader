#!/usr/bin/env bash
# SPDX-License-Identifier: 0BSD
# https://github.com/DLehenbauer/pico-sdcard-bootloader
#
# Rebuild the Pico bootloader distributed in dist/bootloader.uf2.

set -e

# Resolve the project directory independently of the caller.
cd -- "$(dirname -- "$0")"

# Build the production image without discarding other configurations.
cmake --preset rp2040_release "-DBOOTLOADER_CONFIG_FILE=$PWD/config.cmake"
cmake --build --preset rp2040_release

# Refresh the distributed example only after a successful build.
mkdir -p dist
cmake -E copy_if_different build/rp2040_release/src/boot3/bootloader.uf2 dist/bootloader.uf2
