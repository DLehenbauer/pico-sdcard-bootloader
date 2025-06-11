#!/bin/bash

set -e

rm -rf build
rm ./dist/bootloader.uf2 || true

cmake --preset rp2040_release
cmake --build --preset rp2040_release

mkdir -p dist
cp /workspaces/rp2040-sdcard-bootloader/build/rp2040_release/src/boot3/bootloader.uf2 ./dist
