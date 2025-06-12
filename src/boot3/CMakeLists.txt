pico_sdk_init()

add_executable(${PROJECT_NAME}
    diag.c
    flash.c
    interval_set.c
    main.c
    prog.c
    vector_into_flash.S
    transport.c
    vector_table.c
)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory("../../ext/no-OS-FatFS-SD-SPI-RPi-Pico/FatFs_SPI" ${PROJECT_NAME})

# pull in common dependencies
target_link_libraries(${PROJECT_NAME} 
    boot_uf2_headers
    FatFs_SPI
    hardware_flash 
    hardware_timer 
    pico_multicore
    pico_stdlib 
)

pico_set_binary_type(${PROJECT_NAME} default)
pico_set_linker_script(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/memmap.ld)

# print elf size
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND arm-none-eabi-size $<TARGET_FILE:${PROJECT_NAME}>
    COMMENT "Printing binary size..."
)

target_compile_definitions(${PROJECT_NAME} PUBLIC
    PICO_XOSC_STARTUP_DELAY_MULTIPLIER=64  # Increase XOSC startup delay
    NO_PICO_LED                            # Prevent FatFs_SPI from using the LED
    BOOTLOADER_SIZE=${BOOTLOADER_SIZE}
    BOOTLOADER_USE_LED=${BOOTLOADER_USE_LED}
    BOOTLOADER_USE_UART=${BOOTLOADER_USE_UART}
    BOOTLOADER_UART=${BOOTLOADER_UART}
    BOOTLOADER_UART_TX_PIN=${BOOTLOADER_UART_TX_PIN}
    BOOTLOADER_UART_RX_PIN=${BOOTLOADER_UART_RX_PIN}
    BOOTLOADER_UART_BAUD_RATE=${BOOTLOADER_UART_BAUD_RATE}
    BOOTLOADER_SD_SPI=${BOOTLOADER_SD_SPI}
    BOOTLOADER_SD_SPI_SCK_PIN=${BOOTLOADER_SD_SPI_SCK_PIN}
    BOOTLOADER_SD_SPI_TX_PIN=${BOOTLOADER_SD_SPI_TX_PIN}
    BOOTLOADER_SD_SPI_RX_PIN=${BOOTLOADER_SD_SPI_RX_PIN}
    BOOTLOADER_SD_SPI_CSN_PIN=${BOOTLOADER_SD_SPI_CSN_PIN}
    BOOTLOADER_SD_DETECT_PIN=${BOOTLOADER_SD_DETECT_PIN}
    BOOTLOADER_SD_USE_DETECT=${BOOTLOADER_SD_USE_DETECT}
    BOOTLOADER_SD_BAUD_RATE=${BOOTLOADER_SD_BAUD_RATE}
    BOOTLOADER_FIRMWARE_FILENAME="${BOOTLOADER_FIRMWARE_FILENAME}"
)

target_link_options(${PROJECT_NAME} PRIVATE "-Wl,--defsym=BOOTLOADER_SIZE=${BOOTLOADER_SIZE},--defsym=PICO_FLASH_SIZE_BYTES=${PICO_FLASH_SIZE_BYTES}")

# create map/bin/hex file etc.
pico_add_extra_outputs(${PROJECT_NAME})
