if("${CONFIG_STORAGE_SD}" STREQUAL y)
    set(srcs "sd.c" "cv.c")
endif()
if("${CONFIG_STORAGE_FLASH}" STREQUAL y)
    set(srcs "littlefs.c" "cv.c")
endif()

idf_component_register(SRCS ${srcs}
                       PRIV_REQUIRES esp_driver_spi esp_driver_gpio spi_flash
                                     esp_partition vfs fatfs nvs_flash
                       INCLUDE_DIRS "." "../../include" "../../../include"
                       WHOLE_ARCHIVE)
