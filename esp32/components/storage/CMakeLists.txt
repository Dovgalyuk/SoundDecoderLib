if("${CONFIG_STORAGE_SD}" STREQUAL y)
    set(srcs "sd.c")
endif()
if("${CONFIG_STORAGE_FLASH}" STREQUAL y)
    set(srcs "littlefs.c")
endif()

idf_component_register(SRCS ${srcs}
                       PRIV_REQUIRES esp_driver_spi esp_driver_gpio spi_flash esp_partition vfs fatfs
                       INCLUDE_DIRS "." "../../include"
                       WHOLE_ARCHIVE)
