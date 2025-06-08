pico_sdk_init()

# Enable testing functionality
enable_testing()
include(CTest)

# Find GTest
find_package(GTest REQUIRED)
include(GoogleTest)

# Include directories for test code
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src/boot3
    ${CMAKE_SOURCE_DIR}/build/test/generated/pico_base
    ${PICO_SDK_PATH}/src/host/pico_platform/include
    ${PICO_SDK_PATH}/src/common/pico_base_headers/include
    ${PICO_SDK_PATH}/src/common/boot_uf2_headers/include
    ${PICO_SDK_PATH}/src/rp2_common/hardware_flash/include
    ${PICO_SDK_PATH}/src/rp2040/hardware_regs/include
)

# Add all relevant Pico SDK include directories
include_directories(mocks)

# Define common test flags
set(TEST_COMPILE_DEFS
    PICO_NO_HARDWARE=1  # Add this to disable hardware-specific code
    TESTING=1
    
    BOOTLOADER_SIZE=0x10000
    
    # Because we are compiling with PICO_NO_HARDWARE, we need to provide these definitions.
    PICO_FLASH_SIZE_BYTES=0x200000  # 2MB flash size
    XIP_BASE=0x10000000
    SRAM_BASE=0x20000000
    SRAM_END=0x20042000
)

add_executable(bootloader_tests
    ${CMAKE_SOURCE_DIR}/src/boot3/interval_set.c
    ${CMAKE_SOURCE_DIR}/src/boot3/prog.c
    ${CMAKE_SOURCE_DIR}/src/boot3/vector_table.c
    main.cpp
    test_interval_set.cpp
    test_prog.cpp
)

# Link against GTest and our mock library
target_link_libraries(bootloader_tests
    PRIVATE
    GTest::GTest
    GTest::Main
)

target_compile_definitions(bootloader_tests PRIVATE ${TEST_COMPILE_DEFS})

# For better test reporting, use the automatic test discovery
gtest_discover_tests(bootloader_tests)

# Add a custom target to run all tests
add_custom_target(run_all_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS bootloader_tests
    COMMENT "Running all bootloader tests"
)
