#ifndef PINS_H
#define PINS_H

#include "sdkconfig.h"

#if CONFIG_IDF_TARGET_ESP32
/* Sound */
#   define I2S_BCLK_IO1         26
#   define I2S_WS_IO1           27
#   define I2S_DOUT_IO1         25
/* Motor */
#   define MOTOR_INPUT_V        4
#   define MOTOR_OUTPUT_DIR1    2
#   define MOTOR_OUTPUT_DIR2    15
/* Storage */
#   if CONFIG_STORAGE_SD
#       define SD_PIN_MOSI      15
#       define SD_PIN_MISO      2
#       define SD_PIN_CLK       14
#       define SD_PIN_CS        13
#   endif
#   if CONFIG_STORAGE_FLASH
#       define FLASH_SPI_D0      19
#       define FLASH_SPI_D1      23
#       define FLASH_SPI_D2      22
#       define FLASH_SPI_D3      21
#       define FLASH_SPI_CLK     18
#       define FLASH_SPI_CS      5
#   endif
#elif CONFIG_IDF_TARGET_ESP32S3
/* Sound */
#   define I2S_BCLK_IO1         12
#   define I2S_WS_IO1           13
#   define I2S_DOUT_IO1         11
/* Motor */
#   define MOTOR_INPUT_V        1
#   define MOTOR_OUTPUT_DIR1    2
#   define MOTOR_OUTPUT_DIR2    4
/* Storage */
#   if CONFIG_STORAGE_FLASH
#       define FLASH_SPI_D0      8
#       define FLASH_SPI_D1      15
#       define FLASH_SPI_D2      16
#       define FLASH_SPI_D3      17
#       define FLASH_SPI_CLK     18
#       define FLASH_SPI_CS      7
// PCB
// #       define FLASH_SPI_MOSI    5
// #       define FLASH_SPI_MISO    6
// #       define FLASH_SPI_SCK     7
// #       define FLASH_SPI_SS      8
// #       define FLASH_SPI_WP      9
// #       define FLASH_SPI_HD      10
#   endif
#   if CONFIG_STORAGE_SD
#       define SD_PIN_MOSI      4
#       define SD_PIN_MISO      5
#       define SD_PIN_CLK       2
#       define SD_PIN_CS        8
#   endif
#else
#   error Define pins for peripherals
#endif

#endif
