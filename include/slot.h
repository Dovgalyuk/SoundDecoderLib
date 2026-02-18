#ifndef SLOT_H
#define SLOT_H

#include <stdint.h>
#include <stdbool.h>

typedef struct Schedule Schedule;

#define SLOT_STACK_SIZE 8

typedef struct Slot {
    const Schedule *schedule;
    uint32_t pc;
    uint8_t sp;
    bool flag;
    uint8_t stack[SLOT_STACK_SIZE];
} Slot;

void slot_init(Slot *slot, const Schedule *schedule);
void slot_step(Slot *slot);

#endif
