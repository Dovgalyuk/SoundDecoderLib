#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <inttypes.h>
#include "slot.h"
#include "schedule.h"
#include "bytecode.h"
#include "player.h"
#include "vm.h"
#include "logger.h"

#define DEBUG 0
#define DPRINTF(fmt, ...) \
    do if (DEBUG) printf(fmt, ## __VA_ARGS__); while(0)

static uint16_t read_word(const Schedule *sch, uint32_t *pc)
{
    uint16_t res = sch->script[*pc] | (sch->script[*pc + 1] << 8);
    *pc += 2;
    return res;
}

static uint32_t read_dword(const Schedule *sch, uint32_t *pc)
{
    uint32_t w1 = read_word(sch, pc);
    uint32_t w2 = read_word(sch, pc);
    return w1 | (w2 << 16);
}

uint8_t slot_get_var(Slot *slot, uint16_t addr)
{
    addr -= VAR_LOCAL_START;
    if (addr >= VAR_LOCAL_SIZE) {
        return 0;
    }
    return slot->locals[addr];
}

void slot_set_var(Slot *slot, uint16_t addr, uint8_t val)
{
    addr -= VAR_LOCAL_START;
    if (addr < VAR_LOCAL_SIZE) {
        slot->locals[addr] = val;
    }
}

static int16_t slot_read_mem(Slot *slot, uint16_t addr)
{
    if (addr >= VAR_END) {
        return 0;
    }
    if (addr - VAR_LOCAL_START < VAR_LOCAL_SIZE) {
        return slot_get_var(slot, addr);
    }
    uint8_t v = vm_get_var(addr);
    if (addr >= VAR_GLOBAL_SIGNED_START) {
        return (int8_t)v;
    }
    return v;
}

static void slot_write_mem(Slot *slot, uint16_t addr, uint8_t val)
{
    if (addr >= VAR_END) {
        return;
    }
    if (addr - VAR_LOCAL_START < VAR_LOCAL_SIZE) {
        slot_set_var(slot, addr, val);
    } else {
        vm_set_var(addr, val);
    }
}

static void slot_set_pc(Slot *slot, uint32_t pc)
{
    if (!slot->schedule || pc >= slot->schedule->script_size) {
        slot->error = true;
        return;
    }
    slot->pc = pc;
}

static void slot_call_state(Slot *slot, uint32_t addr)
{
    slot->nextstack[slot->nextsp] = slot->pc;
    slot->nextsp = (slot->nextsp + 1) % NEXT_STACK_SIZE;
    slot_set_pc(slot, addr);
}

static void slot_next_state(Slot *slot, uint32_t addr)
{
    /* TODO: stack not needed anymore ? */
    slot_call_state(slot, addr);
    /* Stop sound if state switch was caused by immediate transition */
    if (slot_get_var(slot, F_PLAYING)) {
        player_abort_slot(slot, slot->subslot);
    }
    /* Reset drive-related variables */
    slot_set_var(slot, F_DRIVELOCK, 0);
    /* Reset sound-related variables */
    slot_set_var(slot, F_RESTORE, 0);
}

static void slot_play(Slot *slot, uint16_t id, uint8_t priority,
    uint8_t volmin, uint8_t volmax, uint8_t delay)
{
    if (slot_get_var(slot, F_PLAYING)) {
        player_abort_slot(slot, slot->subslot);
    }
    play_slot_sound(slot, slot->subslot, id, priority, volmin, volmax, delay);
}

void slot_started_sound(Slot *slot, uint8_t subslot)
{
    uint8_t var = subslot == slot->subslot ? F_PLAYING : F_PLAYING2;
    slot_set_var(slot, var, 1);

    vm_reset_trigger();
}

void slot_finished_sound(Slot *slot, uint8_t subslot)
{
    uint8_t var = subslot == slot->subslot ? F_PLAYING : F_PLAYING2;
    slot_set_var(slot, var, 0);
}

void slot_switch(Slot *slot)
{
    uint8_t pl1 = slot_get_var(slot, F_PLAYING);
    uint8_t pl2 = slot_get_var(slot, F_PLAYING2);
    slot_set_var(slot, F_PLAYING, pl2);
    slot_set_var(slot, F_PLAYING2, pl1);
    slot->subslot = 1 - slot->subslot;
}

void slot_init(Slot *slot, Schedule *schedule)
{
    slot->schedule = schedule;
    slot_reset(slot);
}

void slot_clear(Slot *slot)
{
    if (slot->schedule) {
        free(slot->schedule->name);
    }
    free(slot->schedule);
    slot->schedule = NULL;
}

void slot_reset(Slot *slot)
{
    if (!slot->schedule) {
        return;
    }
    slot->error = false;
    slot->sp = 0;
    slot->flag = 0;
    slot->nextsp = 0;
    slot->subslot = 0;
    memset(slot->locals, 0, sizeof(slot->locals));
    slot->pc = slot->schedule->start;

    slot_set_var(slot, F_FUNCTION,
        !!(slot->schedule->flags & SCHEDULE_FLAG_FORCE));
}

void slot_step(Slot *slot)
{
    if (!slot->schedule || slot->error) {
        return;
    }
    int32_t first = slot->pc;
    uint8_t op = slot->schedule->script[slot->pc++];
    uint8_t oparg;
    uint8_t arg8;
    uint16_t arg16;
    uint32_t arg32;
    DPRINTF("%"PRId32":\t0x%x\t", first, op);
    switch (op) {
    case I_TEST0...I_TEST7:
        oparg = op - I_TEST0;
        arg8 = slot->schedule->script[slot->pc++];
        DPRINTF("TEST%d %d\n", oparg, arg8);
        slot->flag = slot_read_mem(slot, arg8) & (1 << oparg);
        break;
    case I_LOADV:
        arg8 = slot->schedule->script[slot->pc++];
        DPRINTF("LOADV %d\n", arg8);
        slot->stack[slot->sp++] = slot_read_mem(slot, arg8);
        break;
    case I_LOADI:
        arg16 = read_word(slot->schedule, &slot->pc);
        DPRINTF("LOADI %d\n", arg16);
        slot->stack[slot->sp++] = arg16;
        break;
    case I_JUMP:
        arg16 = read_word(slot->schedule, &slot->pc);
        DPRINTF("JUMP %d\n", arg16);
        slot->pc = first + (int16_t)arg16;
        break;
    case I_JUMPF:
        arg16 = read_word(slot->schedule, &slot->pc);
        DPRINTF("JUMPF %d\n", arg16);
        if (!slot->flag) {
            slot->pc = first + (int16_t)arg16;
        }
        break;
    case I_JUMPT:
        arg16 = read_word(slot->schedule, &slot->pc);
        DPRINTF("JUMPT %d\n", arg16);
        if (slot->flag) {
            slot->pc = first + (int16_t)arg16;
        }
        break;
    case I_CONDEQ...I_CONDLE:
        oparg = op - I_COND;
        {
            int16_t op1 = slot->stack[--slot->sp];
            int16_t op2 = slot->stack[--slot->sp];
            DPRINTF("COND%d %d %d\n", oparg, op1, op2);
            switch (oparg) {
            case C_EQ:
                slot->flag = op1 == op2;
                break;
            case C_NE:
                slot->flag = op1 != op2;
                break;
            case C_GT:
                slot->flag = op1 > op2;
                break;
            case C_GE:
                slot->flag = op1 >= op2;
                break;
            case C_LT:
                slot->flag = op1 < op2;
                break;
            case C_LE:
                slot->flag = op1 <= op2;
                break;
            default:
                /* Error */
                DPRINTF("ERROR cond\n");
                break;
            }
        }
        break;
    case I_NEXT:
        arg32 = read_dword(slot->schedule, &slot->pc);
        DPRINTF("NEXT %d\n", (int)arg32);
        slot_next_state(slot, arg32);
        break;
    case I_PLAY:
        {
            uint16_t sample = read_word(slot->schedule, &slot->pc);
            uint8_t priority = slot->schedule->script[slot->pc++];
            uint8_t volmin = slot->schedule->script[slot->pc++];
            uint8_t volmax = slot->schedule->script[slot->pc++];
            uint8_t delay = slot->schedule->script[slot->pc++];
            DPRINTF("PLAY %d\n", sample);
            slot_play(slot, sample, priority, volmin, volmax, delay);
        }
        break;
    case I_FUNC:
        arg8 = slot->schedule->script[slot->pc++];
        DPRINTF("FUNC %d\n", arg8);
        switch (arg8) {
        case FUNC_RAND:
            {
                uint8_t min = slot->stack[--slot->sp];
                uint8_t max = slot->stack[--slot->sp];
                int x = min + rand() % (max + 1 - min);
                slot->stack[slot->sp++] = x;
            }
            break;
        default:
            DPRINTF("ERROR func\n");
            /* Error */
            break;
        }
        break;
    case I_STOREV:
        arg8 = slot->schedule->script[slot->pc++];
        DPRINTF("STOREV %d\n", arg8);
        slot_write_mem(slot, arg8, slot->stack[--slot->sp]);
        break;
    case I_SET0...I_SET7:
        {
            oparg = op - I_SET0;
            arg8 = slot->schedule->script[slot->pc++];
            DPRINTF("SET%d %d\n", oparg, arg8);
            uint8_t tmp = slot_read_mem(slot, arg8);
            tmp |= 1 << oparg;
            slot_write_mem(slot, arg8, tmp);
        }
        break;
    case I_RESET0...I_RESET7:
        {
            oparg = op - I_RESET0;
            arg8 = slot->schedule->script[slot->pc++];
            DPRINTF("RESET%d %d\n", oparg, arg8);
            uint8_t tmp = slot_read_mem(slot, arg8);
            tmp &= ~(1 << oparg);
            slot_write_mem(slot, arg8, tmp);
        }
        break;
    case I_ADD:
        {
            DPRINTF("ADD\n");
            uint8_t op1 = slot->stack[--slot->sp];
            uint8_t op2 = slot->stack[--slot->sp];
            slot->stack[slot->sp++] = op1 + op2;
        }
        break;
    case I_SUB:
        {
            DPRINTF("SUB\n");
            uint8_t op1 = slot->stack[--slot->sp];
            uint8_t op2 = slot->stack[--slot->sp];
            slot->stack[slot->sp++] = op1 - op2;
        }
        break;
    case I_SWITCH:
        {
            DPRINTF("SWITCH\n");
            slot_switch(slot);
        }
        break;
    case I_RET:
        DPRINTF("RET\n");
        slot->nextsp = (NEXT_STACK_SIZE + slot->nextsp - 1) % NEXT_STACK_SIZE;
        slot->pc = slot->nextstack[slot->nextsp];
        //printf("return to %d\n", (int)slot->pc);
        break;
    case I_CALL:
        arg32 = read_dword(slot->schedule, &slot->pc);
        DPRINTF("CALL %d\n", (int)arg32);
        slot_call_state(slot, arg32);
        break;
    default:
        /* Error */
        DPRINTF("ERROR instr\n");
        break;
    }
}
