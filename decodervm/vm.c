#include <stdio.h>
#include <stdlib.h>
#include "vm.h"
#include "slot.h"
#include "schedule.h"
#include "audio.h"
#include "utils.h"
#include "cv.h"

#define INSTRUCTIONS_PER_TICK 100

static Slot slots[VM_SLOTS];
static uint8_t memory[VAR_GLOBAL_SIZE];
static bool trigger_set;

void vm_set_var(uint16_t addr, uint8_t val)
{
    addr -= VAR_GLOBAL_START;
    if (addr < VAR_GLOBAL_SIZE) {
        memory[addr] = val;
    }
}

uint8_t vm_get_var(uint16_t addr)
{
    addr -= VAR_GLOBAL_START;
    if (addr >= VAR_GLOBAL_SIZE) {
        return 0;
    }
    return memory[addr];
}

bool vm_load_slot(FILE *f)
{
    uint8_t slot;
    char *name = NULL;
    Schedule *sch = NULL;
    if (!file_read_uint8(f, &slot)) {
        printf("error %d\n", __LINE__);
        goto error;
    }
    if (slot >= VM_SLOTS || slots[slot].schedule) {
        printf("error %d\n", __LINE__);
        goto error;
    }
    if (!file_read_string(f, &name)) {
        printf("error %d\n", __LINE__);
        goto error;
    }
    uint8_t volume;
    if (!file_read_uint8(f, &volume)) {
        printf("error %d\n", __LINE__);
        goto error;
    }
    uint32_t init, length;
    if (!file_read_uint32(f, &init)) {
        printf("error %d\n", __LINE__);
        goto error;
    }
    if (!file_read_uint32(f, &length)) {
        printf("error %d\n", __LINE__);
        goto error;
    }
    sch = malloc(sizeof(Schedule) + length);
    if (!sch) {
        printf("error %d\n", __LINE__);
        goto error;
    }
    sch->name = name;
    sch->volume = volume;
    sch->start = init;
    sch->script_size = length;
    if (fread(sch->script, 1, length, f) != length) {
        printf("error %d\n", __LINE__);
        goto error;
    }
    //printf("Loading slot %d (%s) with %d bytes\n", slot, sch->name, sch->script_size);
    slot_init(&slots[slot], sch);
    return true;
error:
    free(name);
    free(sch);
    return false;
}

void vm_set_slot_var(uint8_t id, uint16_t addr, uint8_t val)
{
    if (id < VM_SLOTS) {
        slot_set_var(&slots[id], addr, val);
    }
}

uint8_t vm_get_slot_var(uint8_t id, uint16_t addr)
{
    if (id >= VM_SLOTS) {
        return 0;
    }
    return slot_get_var(&slots[id], addr);
}

void vm_tick(uint32_t t)
{
    static int32_t trigger_time;
    /* Set trigger */
    //trigger_time += t;
    // project mogul
    // uint32_t period = 950 - vm_get_var(V_SPEED) * 3;
    // project mogul2
    int32_t period = cv_read(CV_CHUFF_PERIOD) * 10
                     - vm_get_var(V_SPEED) * cv_read(CV_CHUFF_SPEEDUP) / 10;
    int32_t min = cv_read(CV_CHUFF_MIN_PERIOD);
    if (period < min) {
        /* For highest speed need not to be prototypical */
        period = min;
    }
    trigger_time += t;
    if (trigger_time >= period) {
        trigger_time -= period;
        trigger_set = true;
    }

    static uint32_t clock_time;
    clock_time += t;
    while (clock_time >= 256) {
        clock_time -= 256;
        /* Decrement timers */
        for (int i = 0 ; i < VM_SLOTS ; ++i) {
            if (!slots[i].schedule) {
                continue;
            }
            uint8_t v = slot_get_var(&slots[i], V_TIMER_1_256MS);
            if (v) {
                slot_set_var(&slots[i], V_TIMER_1_256MS, v - 1);
            }
            v = slot_get_var(&slots[i], V_TIMER_2_256MS);
            if (v) {
                slot_set_var(&slots[i], V_TIMER_2_256MS, v - 1);
            }
        }
    }
    /* Run instructions */
    for (int j = 0 ; j < INSTRUCTIONS_PER_TICK ; ++j) {
        // trigger_time += 1;
        // if (trigger_time >= period) {
        //     trigger_time -= period;
        //     trigger_set = true;
        // }
        /* Trigger at any speed to allow correct emergency stop */
        vm_set_var(F_TRIGGER, trigger_set);

        for (int i = 0 ; i < VM_SLOTS ; ++i) {
            slot_step(&slots[i]);
        }
    }
}

void vm_reset_trigger(void)
{
    trigger_set = false;
}

bool vm_has_drivelock(void)
{
    for (int i = 0 ; i < VM_SLOTS ; ++i) {
        if (!slots[i].schedule) {
            continue;
        }
        if (slot_get_var(&slots[i], F_DRIVELOCK)) {
            return true;
        }
    }
    return false;
}

void vm_clear(void)
{
    for (int i = 0 ; i < VM_SLOTS ; ++i) {
        slot_clear(&slots[i]);
    }
}

void vm_reset(void)
{
    trigger_set = false;
    for (int i = 0 ; i < VM_SLOTS ; ++i) {
        slot_reset(&slots[i]);
    }
}

void vm_set_function_key(uint8_t f, bool v)
{
    if (f < VM_FUNCTION_KEYS) {
        vm_set_var(F_KEY0 + f, v);
    }
}

bool vm_get_function_key(uint8_t f)
{
    if (f < VM_FUNCTION_KEYS) {
        return vm_get_var(F_KEY0 + f);
    }
    return false;
}
