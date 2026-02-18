#include <assert.h>
#include "vm.h"
#include "slot.h"

#define VM_SLOTS        64
#define VM_MEM_SIZE     0x100

static Slot slots[VM_SLOTS];
static uint8_t memory[VM_MEM_SIZE];

void vm_memory_write(uint16_t addr, uint8_t val)
{
    assert(addr < VM_MEM_SIZE);
    memory[addr] = val;
}

uint8_t vm_memory_read(uint16_t addr)
{
    assert(addr < VM_MEM_SIZE);
    return memory[addr];
}

