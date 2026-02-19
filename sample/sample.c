#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <memory.h>
#include <time.h>
#include "schedule.h"
#include "slot.h"
#include "variables.h"
#include "vm.h"
#include "player.h"

extern Schedule sch;

int main(int argc, char **argv)
{
    if (argc < 2) {
        printf("Usage: %s <output.wav>\n", argv[0]);
        return 1;
    }
    srand(time(NULL));

    player_init(argv[1]);

    // vm_set_var(V_SPEED, 50);
    vm_set_var(V_SPEED_REQUEST, 50);
    // vm_set_var(V_ACCEL, 30);
    // vm_set_var(F_TRIGGER, 1);

    vm_load_slot(1, &sch);
    /* Start playing */
    vm_set_slot_var(1, F_FUNCTION, 1);

    for (int i = 0 ; i < 1000 ; ++i) {
        //printf("%d : %d\n", i, slot.pc);
        //slot_step(&slot);
        vm_tick();
    }
    player_clear();
}
