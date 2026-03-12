#ifndef CV_H
#define CV_H

#include <stdint.h>

/* Standard */
#define CV_VSTART               2
#define CV_ACCELERATION         3
#define CV_DECELERATION         4

/* Custom 47-64 */
#define CV_SWITCH               49
#define CV_SOUND_VOLUME         63

/* Custom 112-256 */

#define CV_MAX                  255

uint8_t cv_read(uint16_t id);
void cv_write(uint16_t id, uint8_t value);

#endif
