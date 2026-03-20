#ifndef PINS_H
#define PINS_H

#include "sdkconfig.h"

#if CONFIG_IDF_TARGET_ESP32
/* Sound */
#   define I2S_BCLK_IO1         26
#   define I2S_WS_IO1           27
#   define I2S_DOUT_IO1         25
/* Motor */
#   define MOTOR_OUTPUT_PWM     4
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
#       define FLASH_SPI_MOSI    23
#       define FLASH_SPI_MISO    19
#       define FLASH_SPI_SCK     18
#       define FLASH_SPI_SS      5
#       define FLASH_SPI_WP      22
#       define FLASH_SPI_HD      21
#   endif
#elif CONFIG_IDF_TARGET_ESP32S3
/* Sound */
#   define I2S_BCLK_IO1         36
#   define I2S_WS_IO1           37
#   define I2S_DOUT_IO1         35
/* Motor */
#   define MOTOR_OUTPUT_PWM     5
#   define MOTOR_OUTPUT_DIR1    6
#   define MOTOR_OUTPUT_DIR2    7
// PCB
// #   define MOTOR_SENSE          0
// #   define MOTOR_OUTPUT_DIR1    1
// #   define MOTOR_OUTPUT_DIR2    2
/* Storage */
#   if CONFIG_STORAGE_FLASH
#       define FLASH_SPI_MOSI    4
#       define FLASH_SPI_MISO    5
#       define FLASH_SPI_SCK     3
#       define FLASH_SPI_SS      8
#       define FLASH_SPI_WP      6
#       define FLASH_SPI_HD      7
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
