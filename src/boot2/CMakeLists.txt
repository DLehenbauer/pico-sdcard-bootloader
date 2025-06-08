set(BOOTLOADER_NAME "${PROJECT_NAME}_stage2")

pico_define_boot_stage2(${BOOTLOADER_NAME} "${CMAKE_CURRENT_LIST_DIR}/boot2_w25q080.S")
target_compile_definitions(${BOOTLOADER_NAME} PRIVATE
    BOOTLOADER_SIZE=${BOOTLOADER_SIZE})

# Ensure that our local "exit_from_boot2.S" takes precedence over the one in the Pico SDK.
target_include_directories(${BOOTLOADER_NAME} BEFORE PRIVATE ${CMAKE_CURRENT_LIST_DIR})

pico_set_boot_stage2(${PROJECT_NAME} ${BOOTLOADER_NAME})
