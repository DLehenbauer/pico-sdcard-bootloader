# Default to latest Ubuntu LTS.
# (See https://hub.docker.com/r/microsoft/devcontainers-base)
ARG VARIANT="ubuntu"
FROM mcr.microsoft.com/devcontainers/base:${VARIANT}

# From https://github.com/raspberrypi/pico-sdk/releases
ARG PICO_SDK_VERSION="2.1.1"

# From https://github.com/raspberrypi/openocd/branches
ARG OPENOCD_VERSION="sdk-2.0.0"

# Install apt package dependencies.
#
# For VS Code "xaver.clang-format" extension:
# 
#   clang-format
#
# RP2040 binaries toolchain, plus native toolchain to build 'pioasm' and 'elf2uf2' tools.
# (See Appendix C: https://datasheets.raspberrypi.com/pico/getting-started-with-pico.pdf)
#
#   build-essential
#   check
#   cmake
#   gcc-arm-none-eabi
#   gdb-multiarch
#   libnewlib-arm-none-eabi
#   libstdc++-arm-none-eabi-newlib
#   ninja-build
#
# OpenOCD build dependencies:
#
#   autoconf
#   automake
#   build-essential
#   libftdi-dev
#   libtool
#   libusb-1.0-0-dev
#   pkg-config
#   texinfo
#
# To fix warnings from Cortex-Debug extension:
# (See https://forums.raspberrypi.com/viewtopic.php?t=333146)
#
#   binutils-multiarch
#
# Useful tools:
#
#   minicom
#   ripgrep
#   usbutils

# Use unattended mode for 'apt'
ARG DEBIAN_FRONTEND="noninteractive"
RUN apt-get update && apt-get -y install --no-install-recommends \
    autoconf \
    automake \
    binutils-multiarch \
    bison \
    build-essential \
    check \
    clang-format \
    cmake \
    flex \
    g++ \
    gcc \
    gcc-arm-none-eabi \
    gdb-multiarch \
    gperf \
    lcov \
    libftdi-dev \
    libgtest-dev \
    libnewlib-arm-none-eabi \
    libstdc++-arm-none-eabi-newlib \
    libtool \
    libusb-1.0-0-dev \
    make \
    minicom \
    ninja-build \
    pkg-config \
    ripgrep \
    texinfo \
    usbutils

# Clean apt cache and temporary files
RUN apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# Fix warnings from Cortex-Debug extension
# (See https://forums.raspberrypi.com/viewtopic.php?t=333146)
RUN cd /usr/bin && \
    ln -s /usr/bin/objdump objdump-multiarch && \
    ln -s /usr/bin/nm nm-multiarch

# Install OpenOCD (built from source for RP2040)
RUN cd /tmp && \
    git clone --branch "${OPENOCD_VERSION}" --depth 1 --single-branch https://github.com/raspberrypi/openocd.git && \
    cd "openocd" && \
    ./bootstrap && \
    ./configure && \
    make -j$(nproc) && \
    make install

# Clone Raspberry Pi Pico SDK
# (See https://datasheets.raspberrypi.com/pico/getting-started-with-pico.pdf)
ENV PICO_SDK_PATH="/opt/pico-sdk"
RUN cd /opt && \
    git clone --branch "${PICO_SDK_VERSION}" --depth 1 --single-branch https://github.com/raspberrypi/pico-sdk.git && \
    cd ${PICO_SDK_PATH} && \
    git submodule update --init --recursive --depth 1 --single-branch

# Install 'picotool'
RUN cd /tmp && \
    git clone --depth 1 --single-branch https://github.com/raspberrypi/picotool.git && \
    cd "picotool" && \
    mkdir build && \
    cd build && \
    cmake .. && \
    make -j$(nproc) && \
    make install && \
    cd /tmp && \
    rm -rf picotool
