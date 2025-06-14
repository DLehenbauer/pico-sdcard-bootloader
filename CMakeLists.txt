cmake_minimum_required(VERSION 3.12)

# Must include 'config.cmake' before 'pico_sdk_import.cmake'
if (NOT PICO_PLATFORM STREQUAL "host")
    include("config.cmake")
endif()

# Must include the Pico SDK before 'project()'
include("${PICO_SDK_PATH}/external/pico_sdk_import.cmake")
if (PICO_SDK_VERSION_STRING VERSION_LESS "2.0.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 2.0.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

if (NOT PICO_PLATFORM STREQUAL "host")
    project("bootloader" C CXX ASM)
    add_subdirectory(src)
else()
    project("bootloader_test" C CXX ASM)
    add_subdirectory(test)
endif()
