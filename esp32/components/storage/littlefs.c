/* SPIFFS filesystem example.
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_littlefs.h"
#include "esp_partition.h"
#include "esp_flash.h"
#include "esp_flash_spi_init.h"
#include "driver/gpio.h"
#include "driver/spi_common.h"

#include "storage.h"

#define PIN_SPI_MOSI    23
#define PIN_SPI_MISO    19
#define PIN_SPI_SCK     18
#define PIN_SPI_SS      5
// #define VSPI_IOMUX_PIN_NUM_WP   22
// #define VSPI_IOMUX_PIN_NUM_HD   21

#define PARTITION_LABEL "sound"

static const char *TAG = "LittleFS";

static bool storage_try_littlefs(void)
{
    esp_vfs_littlefs_conf_t conf = {
        .base_path = CONFIG_MOUNT_POINT,
        .partition_label = PARTITION_LABEL,
        .format_if_mount_failed = true,
        .dont_mount = false,
    };
    esp_err_t ret = esp_vfs_littlefs_register(&conf);

    if (ret != ESP_OK) {
        return false;
    }

    size_t total = 0, used = 0;
    ret = esp_littlefs_info(conf.partition_label, &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get LittleFS partition information (%s)", esp_err_to_name(ret));
        esp_littlefs_format(conf.partition_label);
    } else {
        ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
    }
    return true;
}

void storage_register_external_partition(void)
{
    spi_bus_config_t bus_config =
    {
        .mosi_io_num = PIN_SPI_MOSI,
        .miso_io_num = PIN_SPI_MISO,
        .sclk_io_num = PIN_SPI_SCK,
        /* TODO
        .quadwp_io_num = cfg.wp_io_num,
        .quadhd_io_num = cfg.hd_io_num,
        */
        .max_transfer_sz = SPI_MAX_DMA_LEN
    };
    if (spi_bus_initialize(SPI3_HOST, &bus_config, SPI_DMA_CH_AUTO) != ESP_OK) {
        ESP_LOGE(TAG, "Can't initialize external SPI bus");
        return;
    }

    esp_flash_spi_device_config_t flash_config = {
        .host_id = SPI3_HOST,
        .cs_io_num = PIN_SPI_SS,
        .io_mode = SPI_FLASH_DIO,
        .freq_mhz = 40,
        .clock_source = SPI_CLK_SRC_DEFAULT,
    };

    esp_flash_t* ext_flash;
    if (spi_bus_add_flash_device(&ext_flash, &flash_config) != ESP_OK) {
        ESP_LOGE(TAG, "Can't add SPI flash device");
        return;
    }

    if (esp_flash_init(ext_flash) != ESP_OK) {
        ESP_LOGE(TAG, "Can't initialize external flash");
        return;
    }

    uint32_t id;
    ESP_ERROR_CHECK(esp_flash_read_id(ext_flash, &id));
    ESP_LOGI(TAG, "Initialized external flash, size=%" PRIu32 " KB, ID=0x%" PRIx32, ext_flash->size / 1024, id);

    esp_partition_iterator_t it = esp_partition_find(ESP_PARTITION_TYPE_DATA,
        ESP_PARTITION_SUBTYPE_DATA_LITTLEFS, NULL);
    const esp_partition_t *part;
    if (it) {
        part = esp_partition_get(it);
        esp_partition_iterator_release(it);
    } else {
        ESP_LOGI(TAG, "Adding external flash as a partition, label=\"%s\", size=%" PRIu32 " KB",
            PARTITION_LABEL, ext_flash->size / 1024);
        if (esp_partition_register_external(ext_flash, 0, ext_flash->size, PARTITION_LABEL,
            ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_LITTLEFS, &part) != ESP_OK) {
            return;
        }
    }
    /* Pointer is not needed, will format partition with LittleFS library */
}

void storage_init(void)
{
    if (!storage_try_littlefs()) {
        storage_register_external_partition();
        storage_try_littlefs();
    }
}
