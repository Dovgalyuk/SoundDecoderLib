#include <stdio.h>
#include <stdlib.h>
#include "slot.h"
#include "schedule.h"
#include "bytecode.h"
#include "sound.h"
#include "vm.h"

#define DEBUG 0
#define DPRINTF(fmt, ...) \
    do if (DEBUG) printf(fmt, ## __VA_ARGS__); while(0)

static uint16_t read_word(const Schedule *sch, uint32_t *pc)
{
    uint16_t res = sch->script[*pc] | (sch->script[*pc + 1] << 8);
    *pc += 2;
    return res;
}

static uint16_t read_dword(const Schedule *sch, uint32_t *pc)
{
    uint32_t w1 = read_word(sch, pc);
    uint32_t w2 = read_word(sch, pc);
    return w1 | (w2 << 16);
}

void slot_init(Slot *slot, const Schedule *schedule)
{
    slot->schedule = schedule;
    slot->pc = 0;
    slot->sp = 0;
}

void slot_step(Slot *slot)
{
    int32_t first = slot->pc;
    uint8_t op = slot->schedule->script[slot->pc++];
    uint8_t oparg;
    uint8_t arg8;
    uint16_t arg16;
    uint32_t arg32;
    DPRINTF("op = %x\n", op);
    switch (op) {
    case I_TEST0...I_TEST7:
        oparg = op - I_TEST0;
        arg8 = slot->schedule->script[slot->pc++];
        DPRINTF("TEST%d %d\n", oparg, arg8);
        slot->flag = vm_memory_read(arg8) & (1 << oparg);
        break;
    case I_LOADV:
        arg8 = slot->schedule->script[slot->pc++];
        DPRINTF("LOADV %d\n", arg8);
        slot->stack[slot->sp++] = vm_memory_read(arg8);
        break;
    case I_LOADI:
        arg8 = slot->schedule->script[slot->pc++];
        DPRINTF("LOADI %d\n", arg8);
        slot->stack[slot->sp++] = arg8;
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
        DPRINTF("COND%d %d\n", oparg, arg8);
        {
            uint8_t op1 = slot->stack[--slot->sp];
            uint8_t op2 = slot->stack[--slot->sp];
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
        DPRINTF("NEXT %d\n", arg32);
        slot->pc = arg32;
        break;
    case I_WAIT:
        DPRINTF("WAIT\n");
        /* Do nothing */
        break;
    case I_PLAY:
        arg16 = read_word(slot->schedule, &slot->pc);
        arg8 = slot->schedule->script[slot->pc++];
        DPRINTF("PLAY %d %d\n", arg16, arg8);
        play_sound(arg16);
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
        vm_memory_write(arg8, slot->stack[--slot->sp]);
        break;
    case I_SET0...I_SET7:
        {
            oparg = op - I_SET0;
            arg8 = slot->schedule->script[slot->pc++];
            uint8_t tmp = vm_memory_read(arg8);
            tmp |= 1 << oparg;
            vm_memory_write(arg8, tmp);
        }
        break;
    case I_RESET0...I_RESET7:
        {
            oparg = op - I_RESET0;
            arg8 = slot->schedule->script[slot->pc++];
            uint8_t tmp = vm_memory_read(arg8);
            tmp &= ~(1 << oparg);
            vm_memory_write(arg8, tmp);
        }
        break;
    case I_ADD:
        {
            uint8_t op1 = slot->stack[--slot->sp];
            uint8_t op2 = slot->stack[--slot->sp];
            slot->stack[slot->sp++] = op1 + op2;
        }
        break;
    case I_SUB:
        {
            uint8_t op1 = slot->stack[--slot->sp];
            uint8_t op2 = slot->stack[--slot->sp];
            slot->stack[slot->sp++] = op1 - op2;
        }
        break;
    default:
        /* Error */
        DPRINTF("ERROR instr\n");
        break;
    }
}
