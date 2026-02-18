#ifndef VM_H
#define VM_H

#include <stdint.h>

void vm_memory_write(uint16_t addr, uint8_t val);
uint8_t vm_memory_read(uint16_t addr);

#endif
