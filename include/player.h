#ifndef PLAYER_H
#define PLAYER_H

#include <stdint.h>

typedef struct Slot Slot;

void player_init(void);
void player_clear(void);

void player_abort_slot(Slot *slot, uint8_t subslot);
void play_slot_sound(Slot *slot, uint8_t subslot, uint16_t id,
                     uint8_t priority, uint8_t volmin,
                     uint8_t volmax, uint8_t delay);

#endif
