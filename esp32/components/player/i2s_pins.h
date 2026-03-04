#pragma once

#include "sdkconfig.h"

#ifdef __cplusplus
extern "C" {
#endif

#if CONFIG_IDF_TARGET_ESP32
#define I2S_BCLK_IO1        GPIO_NUM_26     // I2S bit clock io number
#define I2S_WS_IO1          GPIO_NUM_27     // I2S word select io number
#define I2S_DOUT_IO1        GPIO_NUM_25     // I2S data out io number

#elif CONFIG_IDF_TARGET_ESP32P4
#define I2S_BCLK_IO1        GPIO_NUM_45     // I2S bit clock io number
#define I2S_WS_IO1          GPIO_NUM_46     // I2S word select io number
#define I2S_DOUT_IO1        GPIO_NUM_47     // I2S data out io number

#elif CONFIG_IDF_TARGET_ESP32S3
#define I2S_BCLK_IO1        GPIO_NUM_2      // I2S bit clock io number
#define I2S_WS_IO1          GPIO_NUM_3      // I2S word select io number
#define I2S_DOUT_IO1        GPIO_NUM_4      // I2S data out io number

#else
#define I2S_BCLK_IO1        GPIO_NUM_2      // I2S bit clock io number
#define I2S_WS_IO1          GPIO_NUM_3      // I2S word select io number
#define I2S_DOUT_IO1        GPIO_NUM_4      // I2S data out io number

#endif

#ifdef __cplusplus
}
#endif
