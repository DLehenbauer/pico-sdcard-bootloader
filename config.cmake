set(BOOTLOADER_FIRMWARE_FILENAME "firmware.uf2")

# Pico flash size is 2MB (normally set by SDK for supported boards)
# math(EXPR PICO_FLASH_SIZE_BYTES "2 * 1024 * 1024" OUTPUT_FORMAT HEXADECIMAL)

# Reserve 64kB for the bootloader
math(EXPR BOOTLOADER_SIZE "64 * 1024" OUTPUT_FORMAT HEXADECIMAL)

# Enable LED status indicator
set(BOOTLOADER_USE_LED true)

# Enable UART logging
set(BOOTLOADER_USE_UART true)

#  Pico Pin | GPIO      | Adapter Pin | Description               
# ----------|-----------|-------------|---------------------------
#  21       | 16 (RX)   | DO          | Data out (from SD card)   
#  22       | 17        | CS / SS     | Chip select               
#  24       | 18 (SCK)  | SCK         | Serial clock              
#  25       | 19 (TX)   | DI          | Data in (to SD card)      
#  23       | GND       | GND / VSS   | Ground                    
#  36       | 3V3 (Out) | 3V3 / VCC   | Power                     
#  29       | 22 (In)   | CD or DAT3  | Card detect (if used)     

set(BOOTLOADER_SD_SPI_INSTANCE spi0)
set(BOOTLOADER_SD_SPI_SCK_PIN 18)
set(BOOTLOADER_SD_SPI_TX_PIN 19)
set(BOOTLOADER_SD_SPI_RX_PIN 16)
set(BOOTLOADER_SD_SPI_CSN_PIN 17)
set(BOOTLOADER_SD_DETECT_PIN 22)
set(BOOTLOADER_SD_USE_DETECT false)

# set(BOOTLOADER_SD_SPI_INSTANCE spi1)
# set(BOOTLOADER_SD_SPI_SCK_PIN 14)
# set(BOOTLOADER_SD_SPI_TX_PIN 11)
# set(BOOTLOADER_SD_SPI_RX_PIN 12)
# set(BOOTLOADER_SD_SPI_CSN_PIN 9)
# set(BOOTLOADER_SD_DETECT_PIN 8)
# set(BOOTLOADER_SD_USE_DETECT true)

# SD card SPI baud rate: 12.5MHz
set(BOOTLOADER_SD_BAUD_RATE 12500000)
