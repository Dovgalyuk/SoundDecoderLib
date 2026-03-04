#ifndef ENGINE_H
#define ENGINE_H

#include <stdint.h>

void engine_init(void);
void engine_set_throttle(int16_t v);
int16_t engine_get_speed(void);
void engine_tick(uint32_t t);

#endif
