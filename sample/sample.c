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

extern Schedule sch;
Slot slot;
FILE *wav;

uint8_t header[44] = "RIFF\xff\xff\xff\x00WAVEfmt "
    "\x10\x00\x00\x00\x01\x00\x01\x00\x12\x7a\x00\x00\x24\xf4\x00\x00\x02\x00\x10\x00"
    "data\xff\xff\xff\x00";
uint32_t fullsize;

void play_sound(uint16_t id)
{
    printf("play %d\n", id);
    char s[100];
    sprintf(s, "%d.wav", id);
    FILE *f = fopen(s, "rb");
    if (!f) {
        printf("Can't open file %d\n", id);
        exit(1);
    }
    fseek(f, 0, SEEK_END);
    long size = ftell(f) - sizeof(header);
    fseek(f, 0, SEEK_SET);
    uint8_t h[sizeof(header)];
    fread(h, sizeof(h), 1, f);
    uint8_t *buf = malloc(size);
    fread(buf, size, 1, f);
    if (h[0x22] == 8) {
        uint8_t *buf2 = malloc(size * 4);
        for (int i = 0 ; i < size ; ++i) {
            buf2[4 * i] = 0;
            buf2[4 * i + 1] = (int)buf[i] - 0x80;
            buf2[4 * i + 2] = 0;
            buf2[4 * i + 3] = (int)buf[i] - 0x80;
        }
        free(buf);
        buf = buf2;
        size *= 4;
    }
    fullsize += size;
    fwrite(buf, size, 1, wav);
    free(buf);
    fclose(f);
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        printf("Usage: %s <output.wav>\n", argv[0]);
        return 1;
    }
    srand(time(NULL));

    wav = fopen(argv[1], "wb");
    fwrite(header, sizeof(header), 1, wav);

    /* Start playing */
    vm_memory_write(F_FUNCTION, 1);
    vm_memory_write(V_SPEED, 50);
    vm_memory_write(V_SPEED_REQUEST, 50);

    slot_init(&slot, &sch);

    for (int i = 0 ; i < 1000 ; ++i) {
        printf("%d : %d\n", i, slot.pc);
        slot_step(&slot);
    }
    fseek(wav, 4, SEEK_SET);
    uint32_t t = fullsize + sizeof(header) - 8;
    fwrite(&t, 4, 1, wav);
    fseek(wav, sizeof(header) - 4, SEEK_SET);
    fwrite(&fullsize, 4, 1, wav);
    fclose(wav);
}
