#ifndef PINS_H
#define PINS_H

#include "sdkconfig.h"

#if CONFIG_IDF_TARGET_ESP32
#   define I2S_BCLK_IO1         26
#   define I2S_WS_IO1           27
#   define I2S_DOUT_IO1         25
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
#   define I2S_BCLK_IO1         36
#   define I2S_WS_IO1           37
#   define I2S_DOUT_IO1         35
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
