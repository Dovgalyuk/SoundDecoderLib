#include <stdlib.h>
#include "cv.h"
#include "utils.h"

typedef struct CVDesc {
    const uint8_t default_value;
    const char *name;
    const char *description;
    const uint8_t min;
    const uint8_t max;
} CVDesc;

static const CVDesc cv_desc[CV_MAX + 1] = {
    [CV_VSTART] = {130, "Start motor voltage", "Start motor voltage for forward movement 255=VCC", 0, 255},
    [CV_ACCELERATION] = {10, "Acceleration", "0 for acceleration without delay", 0, 255},
    [CV_DECELERATION] = {10, "Deceleration", "0 for slowing down without delay", 0, 255},
    [CV_CHUFF_PERIOD] = {55, "Chuff period", "Steam chuffs period at speed 1 in 10s of milliseconds", 30, 255},
    [CV_CHUFF_SPEEDUP] = {180, "Chuff speedup", "Chuff speedup factor", 0, 255},
    [CV_CHUFF_MIN_PERIOD] = {150, "Minimum chuff period", "Chuff period could not be less at highest speeds", 0, 255},
    [CV_REVERSE_VSTART] = {144, "Reverse start motor voltage", "Start motor voltage for reverse movement 255=VCC", 0, 255},
    [CV_SPEED_TABLE1] = {1, "Speed table step 1", "1-255", 1, 255},
    [CV_SPEED_TABLE2] = {2, "Speed table step 2", "1-255", 1, 255},
    [CV_SPEED_TABLE3] = {4, "Speed table step 3", "1-255", 1, 255},
    [CV_SPEED_TABLE4] = {7, "Speed table step 4", "1-255", 1, 255},
    [CV_SPEED_TABLE5] = {10, "Speed table step 5", "1-255", 1, 255},
    [CV_SPEED_TABLE6] = {14, "Speed table step 6", "1-255", 1, 255},
    [CV_SPEED_TABLE7] = {18, "Speed table step 7", "1-255", 1, 255},
    [CV_SPEED_TABLE8] = {23, "Speed table step 8", "1-255", 1, 255},
    [CV_SPEED_TABLE9] = {28, "Speed table step 9", "1-255", 1, 255},
    [CV_SPEED_TABLE10] = {34, "Speed table step 10", "1-255", 1, 255},
    [CV_SPEED_TABLE11] = {40, "Speed table step 11", "1-255", 1, 255},
    [CV_SPEED_TABLE12] = {47, "Speed table step 12", "1-255", 1, 255},
    [CV_SPEED_TABLE13] = {54, "Speed table step 13", "1-255", 1, 255},
    [CV_SPEED_TABLE14] = {62, "Speed table step 14", "1-255", 1, 255},
    [CV_SPEED_TABLE15] = {70, "Speed table step 15", "1-255", 1, 255},
    [CV_SPEED_TABLE16] = {79, "Speed table step 16", "1-255", 1, 255},
    [CV_SPEED_TABLE17] = {88, "Speed table step 17", "1-255", 1, 255},
    [CV_SPEED_TABLE18] = {98, "Speed table step 18", "1-255", 1, 255},
    [CV_SPEED_TABLE19] = {108, "Speed table step 19", "1-255", 1, 255},
    [CV_SPEED_TABLE20] = {120, "Speed table step 20", "1-255", 1, 255},
    [CV_SPEED_TABLE21] = {133, "Speed table step 21", "1-255", 1, 255},
    [CV_SPEED_TABLE22] = {147, "Speed table step 22", "1-255", 1, 255},
    [CV_SPEED_TABLE23] = {162, "Speed table step 23", "1-255", 1, 255},
    [CV_SPEED_TABLE24] = {178, "Speed table step 24", "1-255", 1, 255},
    [CV_SPEED_TABLE25] = {195, "Speed table step 25", "1-255", 1, 255},
    [CV_SPEED_TABLE26] = {213, "Speed table step 26", "1-255", 1, 255},
    [CV_SPEED_TABLE27] = {233, "Speed table step 27", "1-255", 1, 255},
    [CV_SPEED_TABLE28] = {255, "Speed table step 28", "1-255", 1, 255},
};

static uint8_t cv[CV_MAX + 1];


void cv_init()
{
    for (int i = 0 ; i <= CV_MAX ; ++i) {
        cv_set(i, cv_desc[i].default_value);
    }
}

uint8_t cv_read(cv_addr_t id)
{
    if (id >= CV_MAX) {
        return 0;
    }
    return cv[id];
}

void cv_set(cv_addr_t id, uint8_t value)
{
    if (id >= CV_MAX || value < cv_desc[id].min || value > cv_desc[id].max) {
        return;
    }
    cv[id] = value;
}

const char *cv_name(cv_addr_t id)
{
    if (id >= CV_MAX) {
        return NULL;
    }
    return cv_desc[id].name;
}

const char *cv_description(cv_addr_t id)
{
    if (id >= CV_MAX) {
        return NULL;
    }
    return cv_desc[id].description;
}

bool cv_load(FILE *f)
{
    uint16_t count;
    if (!file_read_uint16(f, &count)) {
        return false;
    }
    while (count--) {
        cv_addr_t id;
        if (!file_read_uint16(f, &id)) {
            return false;
        }
        uint8_t value;
        if (!file_read_uint8(f, &value)) {
            return false;
        }
        cv_set(id, value);
    }
    return true;
}
