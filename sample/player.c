#include <stdio.h>
#include <stdlib.h>
#include "player.h"
#include "slot.h"

/*
    Dumps all the played files into the single WAV file.
    Does not support mixing of the sounds.
*/

static FILE *wav;
static uint32_t fullsize;

static const uint8_t header[44] = "RIFF\xff\xff\xff\x00WAVEfmt "
    "\x10\x00\x00\x00\x01\x00\x01\x00\x12\x7a\x00\x00\x24\xf4\x00\x00\x02\x00\x10\x00"
    "data\xff\xff\xff\x00";

void player_init(void *opaque)
{
    const char *filename = opaque;
    wav = fopen(filename, "wb");
    fwrite(header, sizeof(header), 1, wav);
}

void player_clear(void)
{
    fseek(wav, 4, SEEK_SET);
    uint32_t t = fullsize + sizeof(header) - 8;
    fwrite(&t, 4, 1, wav);
    fseek(wav, sizeof(header) - 4, SEEK_SET);
    fwrite(&fullsize, 4, 1, wav);
    fclose(wav);
}

void player_abort_slot(Slot *slot)
{
}

void play_slot_sound(Slot *slot, uint16_t id, uint8_t priority)
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
