set(srcs engine.c drv8235.c)

idf_component_register(SRCS ${srcs}
                       PRIV_REQUIRES decodervm esp_driver_ledc esp_driver_gpio esp_adc esp_driver_i2c
                       INCLUDE_DIRS "../../../include" "../../include")
