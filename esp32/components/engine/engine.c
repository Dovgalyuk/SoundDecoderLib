/*
CVs

CV 2    Start volt
CV 3	Acceleration Rate (full speed for 0.896*CV3 seconds)
CV 4	Deceleration Rate

*/
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "driver/gpio.h"

#include "cv.h"
#include "engine.h"
#include "vm.h"
#include "variables.h"
#include "pins.h"

#define MOTOR_SPEED_MODE     LEDC_LOW_SPEED_MODE
#define MOTOR_LEDC_CHANNEL1  LEDC_CHANNEL_0
#define MOTOR_LEDC_CHANNEL2  LEDC_CHANNEL_1
#define MOTOR_PWM_FREQUENCY  40000
#define MOTOR_PWM_RESOLUTION LEDC_TIMER_8_BIT
#define MOTOR_PWM_MAX        255

static void engine_task(void *args)
{
    while (true) {
        /* Update speed */
        uint8_t speed = engine_get_speed();
        uint8_t s1 = MOTOR_PWM_MAX;
        uint8_t s2 = MOTOR_PWM_MAX;
        if (speed) {
            bool dir = engine_get_direction();
            uint16_t min = cv_read(CV_VSTART);
            if (!dir) {
                min = cv_read(CV_REVERSE_VSTART);
            }
            uint8_t range = 255 - min;
            // Drive 0 output as PWM
            uint8_t pwm = MOTOR_PWM_MAX - (min + (range * speed) / MOTOR_PWM_MAX);
            // FWD 10
            // REV 01
            if (dir) {
                s2 = pwm;
            } else {
                s1 = pwm;
            }
        }
        ledc_set_duty(MOTOR_SPEED_MODE, MOTOR_LEDC_CHANNEL1, s1);
        ledc_set_duty(MOTOR_SPEED_MODE, MOTOR_LEDC_CHANNEL2, s2);
        ledc_update_duty(MOTOR_SPEED_MODE, MOTOR_LEDC_CHANNEL1);
        ledc_update_duty(MOTOR_SPEED_MODE, MOTOR_LEDC_CHANNEL2);

        /* Update smoke */

        /* Update LEDs */

        /* Wait */
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void engine_init(void)
{
    /* Setup PWM pins for motor driver */
    ledc_timer_config_t ledc_timer1 = {
        .speed_mode       = MOTOR_SPEED_MODE,
        .duty_resolution  = MOTOR_PWM_RESOLUTION,
        .timer_num        = LEDC_TIMER_0,
        .freq_hz          = MOTOR_PWM_FREQUENCY,
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer1));
    ledc_channel_config_t ledc_channel1 = {
        .speed_mode     = MOTOR_SPEED_MODE,
        .channel        = MOTOR_LEDC_CHANNEL1,
        .timer_sel      = LEDC_TIMER_0,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = MOTOR_OUTPUT_DIR1,
        .duty           = MOTOR_PWM_MAX,
        .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel1));

    ledc_timer_config_t ledc_timer2 = {
        .speed_mode       = MOTOR_SPEED_MODE,
        .duty_resolution  = MOTOR_PWM_RESOLUTION,
        .timer_num        = LEDC_TIMER_1,
        .freq_hz          = MOTOR_PWM_FREQUENCY,
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer2));
    ledc_channel_config_t ledc_channel2 = {
        .speed_mode     = MOTOR_SPEED_MODE,
        .channel        = MOTOR_LEDC_CHANNEL2,
        .timer_sel      = LEDC_TIMER_1,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = MOTOR_OUTPUT_DIR2,
        .duty           = MOTOR_PWM_MAX,
        .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel2));

    /* Initialize other pins for motor driver */
    // gpio_config_t io_conf = {
    //     .intr_type = GPIO_INTR_DISABLE,
    //     .mode = GPIO_MODE_OUTPUT,
    //     .pin_bit_mask = (1ULL << MOTOR_OUTPUT_DIR1) | (1ULL << MOTOR_OUTPUT_DIR2),
    //     .pull_down_en = 0,
    //     .pull_up_en = 0,
    // };
    // gpio_config(&io_conf);
    /* Main task for controlling speed */
    xTaskCreatePinnedToCore(engine_task, "engine_task", 2560, NULL, 5, NULL, 0);
}
