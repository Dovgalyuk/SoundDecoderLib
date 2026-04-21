#ifndef PINS_H
#define PINS_H

#include "sdkconfig.h"

#if CONFIG_IDF_TARGET_ESP32S3
/* Sound */
#   define I2S_BCLK_IO1         12
#   define I2S_WS_IO1           13
#   define I2S_DOUT_IO1         11
/* Motor */
#   define MOTOR_INPUT_V        1
#   define MOTOR_OUTPUT_DIR1    2
#   define MOTOR_OUTPUT_DIR2    4
/* Physical outputs */
#   define PHYS_OUTPUT_SMOKE      47
#   define PHYS_OUTPUT_FWD_LIGHT  48
#   define PHYS_OUTPUT_BACK_LIGHT 42
/* Storage */
#   define FLASH_SPI_D0         8
#   define FLASH_SPI_D1         15
#   define FLASH_SPI_D2         16
#   define FLASH_SPI_D3         17
#   define FLASH_SPI_CLK        18
#   define FLASH_SPI_CS         7
#else
#   error Define pins for peripherals
#endif

#endif
