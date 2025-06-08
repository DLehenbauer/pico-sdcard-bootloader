cmake_minimum_required(VERSION 3.12)

set(PICO_PLATFORM rp2040)
set(PICO_BOARD pico)

# Initialize the SDK
pico_sdk_init()

add_compile_options(-Wall -Wextra -Wnull-dereference -Wno-unused-parameter -Wno-unused-function)

# Note that 'boot3' must be added before 'boot2'.
add_subdirectory(boot3)
add_subdirectory(boot2)
