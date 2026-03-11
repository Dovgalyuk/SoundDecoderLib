#include "cv.h"

static uint8_t cv[CV_MAX + 1] = {
    [CV_VSTART] = 128,
    [CV_ACCELERATION] = 2,
    [CV_DECELERATION] = 2,
};

static const char *cv_names[CV_MAX + 1] = {
    /* Not named items are not used yet */
    [CV_VSTART] = "Start motor voltage",
    [CV_ACCELERATION] = "Acceleration",
    [CV_DECELERATION] = "Deceleration",
};

uint8_t cv_read(uint16_t id)
{
    if (id >= CV_MAX) {
        return 0;
    }
    return cv[id];
}
