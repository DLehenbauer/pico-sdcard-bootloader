cmake_minimum_required(VERSION 3.12)

# Initialize the SDK
pico_sdk_init()

# Ensure board-header expressions are normalized before using the physical sizes
# in both stages.
math(EXPR PICO_FLASH_SIZE_BYTES "${PICO_FLASH_SIZE_BYTES}")
math(EXPR BOOTLOADER_SIZE "${BOOTLOADER_SIZE}")

add_compile_options(-Wall -Wextra -Wnull-dereference -Wno-unused-parameter -Wno-unused-function)

# Note that 'boot3' must be added before 'boot2'.
add_subdirectory(boot3)
add_subdirectory(boot2)
