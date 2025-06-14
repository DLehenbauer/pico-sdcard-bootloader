# Adjust the PICO_BOARD type as needed below. This will select the correct
# stage 2 bootloader for the flash ram on the target as well as the default
# UART and SPI configurations.
#
# Note: PICO_BOARD must be set before including pico_sdk_import.cmake
set(PICO_BOARD pico)

# Name of the '.uf2' firmware file to write to flash.
set(BOOTLOADER_FIRMWARE_FILENAME "firmware.uf2")

# Typically, PICO_FLASH_SIZE_BYTES is set by the SDK based on the board type.
# math(EXPR PICO_FLASH_SIZE_BYTES "2 * 1024 * 1024" OUTPUT_FORMAT HEXADECIMAL)

# Reserve 64kB for the bootloader
math(EXPR BOOTLOADER_SIZE "64 * 1024" OUTPUT_FORMAT HEXADECIMAL)

# Configure the LED status indicator
set(BOOTLOADER_USE_LED true)
set(BOOTLOADER_LED_PIN "PICO_DEFAULT_LED_PIN")

# Configure UART logging.
set(BOOTLOADER_USE_UART false)
set(BOOTLOADER_UART "PICO_DEFAULT_UART")
set(BOOTLOADER_UART_TX_PIN "PICO_DEFAULT_UART_TX_PIN")
set(BOOTLOADER_UART_RX_PIN "PICO_DEFAULT_UART_RX_PIN")
set(BOOTLOADER_UART_BAUD_RATE "PICO_DEFAULT_UART_BAUD_RATE")

#  Pico Pin | GPIO      | Adapter Pin | Description               
# ----------|-----------|-------------|---------------------------
#  21       | 16 (RX)   | DO          | Data out (from SD card)
#  22       | 17        | CS / SS     | Chip select
#  24       | 18 (SCK)  | SCK         | Serial clock
#  25       | 19 (TX)   | DI          | Data in (to SD card)
#  23       | GND       | GND / VSS   | Ground
#  36       | 3V3 (Out) | 3V3 / VCC   | Power
#  29       | 22 (In)   | CD or DAT3  | Card detect (if used)

set(BOOTLOADER_SD_SPI "PICO_DEFAULT_SPI")
set(BOOTLOADER_SD_SPI_SCK_PIN "PICO_DEFAULT_SPI_SCK_PIN")
set(BOOTLOADER_SD_SPI_TX_PIN "PICO_DEFAULT_SPI_TX_PIN")
set(BOOTLOADER_SD_SPI_RX_PIN "PICO_DEFAULT_SPI_RX_PIN")
set(BOOTLOADER_SD_SPI_CSN_PIN "PICO_DEFAULT_SPI_CSN_PIN")

# Optionally use the card detect pin to determine if an SD card is present.
# If not used, the bootloader will poll for the SD card by seeing if it responds
# to commands.
set(BOOTLOADER_SD_USE_DETECT false)
set(BOOTLOADER_SD_DETECT_PIN 22)

# SD card SPI baud rate: 12.5MHz
set(BOOTLOADER_SD_BAUD_RATE 12500000)
