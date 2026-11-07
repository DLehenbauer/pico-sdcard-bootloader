# Default to latest Ubuntu LTS.
# (See https://hub.docker.com/r/microsoft/devcontainers-base)
ARG VARIANT="ubuntu"
ARG PLATFORM="linux/amd64"
FROM --platform=${PLATFORM} mcr.microsoft.com/devcontainers/base:${VARIANT}
SHELL ["/bin/bash", "-o", "pipefail", "-c"]

# Keep costly, stable installations before smaller or frequently edited inputs.
# Download, install and clean each temporary source tree in one layer.
# Declare tool versions at first use to avoid invalidating unrelated earlier layers.

# Use unattended mode for 'apt'
ARG DEBIAN_FRONTEND="noninteractive"

# Install Kitware APT repository so CMake is newer than Ubuntu default.
RUN apt-get update && apt-get -y install --no-install-recommends \
    ca-certificates \
    gpg \
    wget && \
    wget -qO - https://apt.kitware.com/keys/kitware-archive-latest.asc | \
    gpg --dearmor - | tee /usr/share/keyrings/kitware-archive-keyring.gpg >/dev/null && \
    . /etc/os-release && \
    echo "deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ ${UBUNTU_CODENAME} main" \
    | tee /etc/apt/sources.list.d/kitware.list >/dev/null && \
    apt-get update && \
    apt-get -y install --no-install-recommends kitware-archive-keyring && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# Install only the bootloader's toolchains, tests, debugger and hardware tools.
ARG CMAKE_VERSION="3.*"
RUN apt-get update && apt-get -y install --no-install-recommends \
    autoconf \
    automake \
    binutils-multiarch \
    build-essential \
    clang-format \
    cmake=${CMAKE_VERSION} \
    cmake-data=${CMAKE_VERSION} \
    gcc-arm-none-eabi \
    gdb-multiarch \
    git-lfs \
    libftdi-dev \
    libgtest-dev \
    libjim-dev \
    libnewlib-arm-none-eabi \
    libstdc++-arm-none-eabi-newlib \
    libtool \
    libusb-1.0-0-dev \
    minicom \
    ninja-build \
    pkg-config \
    python3 \
    ripgrep \
    texinfo \
    usbutils && \
    cmake --version && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# Build the packaged GTest sources in Debug, like the tests that link them.
RUN cmake -S /usr/src/googletest -B /tmp/gtest-build -G Ninja \
        -DCMAKE_BUILD_TYPE=Debug -DBUILD_GMOCK=OFF && \
    cmake --build /tmp/gtest-build --parallel && \
    cmake --install /tmp/gtest-build && \
    rm -rf /tmp/gtest-build

# Fix warnings from Cortex-Debug extension
# (See https://forums.raspberrypi.com/viewtopic.php?t=333146)
RUN cd /usr/bin && \
    ln -s /usr/bin/objdump objdump-multiarch && \
    ln -s /usr/bin/nm nm-multiarch

# Match EconoPET's RP2040 OpenOCD and its glibc 2.42+ compatibility option.
ARG OPENOCD_VERSION="sdk-2.3.0"
RUN git clone --branch "${OPENOCD_VERSION}" --depth 1 --single-branch \
    https://github.com/raspberrypi/openocd.git /tmp/openocd && \
    cd /tmp/openocd && \
    ./bootstrap && \
    ./configure --disable-werror && \
    make -j"$(nproc)" && \
    make install && \
    cd /tmp && \
    rm -rf openocd

# Install the SDK before picotool, which uses it during its build.
# (See https://datasheets.raspberrypi.com/pico/getting-started-with-pico.pdf)
ARG PICO_SDK_VERSION="2.3.0"
ENV PICO_SDK_PATH="/opt/pico-sdk"
RUN git clone --branch "${PICO_SDK_VERSION}" --depth 1 --single-branch \
    https://github.com/raspberrypi/pico-sdk.git "${PICO_SDK_PATH}" && \
    git -C "${PICO_SDK_PATH}" submodule update --init --recursive --depth 1 --single-branch

# Pin picotool to the matching SDK release instead of the default branch.
RUN git clone --branch "${PICO_SDK_VERSION}" --depth 1 --single-branch \
    https://github.com/raspberrypi/picotool.git /tmp/picotool && \
    cmake -S /tmp/picotool -B /tmp/picotool/build -G Ninja \
        -DCMAKE_BUILD_TYPE=RelWithDebInfo && \
    cmake --build /tmp/picotool/build --parallel && \
    cmake --install /tmp/picotool/build && \
    rm -rf /tmp/picotool
