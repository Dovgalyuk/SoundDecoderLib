#include "cv.h"

typedef struct CV {
    uint8_t value;
    const char *name;
    const char *description;
} CV;

static CV cv[CV_MAX + 1] = {
    [CV_VSTART] = {128, "Start motor voltage", "0-255 = 0V-VCC"},
    [CV_ACCELERATION] = {2, "Acceleration", "0 for acceleration without delay"},
    [CV_DECELERATION] = {2, "Deceleration", "0 for slowing down without delay"},
};

uint8_t cv_read(uint16_t id)
{
    if (id >= CV_MAX) {
        return 0;
    }
    return cv[id].value;
}

void cv_write(uint16_t id, uint8_t value)
{
    if (id >= CV_MAX) {
        return;
    }
    cv[id].value = value;
}
