#ifndef SCHEDULE_H
#define SCHEDULE_H

#include <stdint.h>

typedef struct Schedule {
#ifdef EMULATOR
    const char *name;
#endif
    uint32_t script_size;
    uint8_t script[];
} Schedule;

#endif
