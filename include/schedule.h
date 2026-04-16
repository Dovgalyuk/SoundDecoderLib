#ifndef SCHEDULE_H
#define SCHEDULE_H

#include <stdint.h>

#define SCHEDULE_FLAG_FORCE 0x01

typedef struct Schedule {
    uint32_t start;
    uint32_t script_size;
    uint8_t volume;
    uint8_t flags;
    char *name;
    uint8_t script[];
} Schedule;

#endif
