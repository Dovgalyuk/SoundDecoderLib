/*
CVs

acceleration speed
slowdown speed
max speed
speed step

CV 2    Start volt
CV 3	Acceleration Rate (full speed for 0.896*CV3 seconds)
CV 4	Deceleration Rate

Functions

F1 - start engine
F6 - shunting mode (change speed without delay)
F7 - half speed

*/
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "engine.h"
#include "vm.h"
#include "variables.h"

static void engine_task(void *args)
{
    while (true) {
        /* update hardware parameters */
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void engine_init(void)
{
    /* Main task for controlling speed */
    xTaskCreatePinnedToCore(engine_task, "engine_task", 2560, NULL, 5, NULL, 1);
}
