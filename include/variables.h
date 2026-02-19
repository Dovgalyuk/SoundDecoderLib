#ifndef VARIABLES_H
#define VARIABLES_H

#define VAR_LOCAL_START         0x00
#define VAR_LOCAL_SIZE          0x40
#define VAR_GLOBAL_START        0x40
#define VAR_GLOBAL_SIZE         0xC0
#define VAR_GLOBAL_SIGNED_START 0xE0
#define VAR_END                 0x100

/* Local flags and variables */
#define F_FUNCTION      0x00
#define F_PLAYING       0x01
#define V_TIMER_1_256MS 0x02
#define V_TIMER_2_256MS 0x03
#define V_USER_1        0x04
#define V_USER_2        0x05
#define V_USER_3        0x06
#define V_USER_4        0x07
/* Set to prevent movement while playing */
#define F_DRIVELOCK     0x08
/* Restore sound when re-enabling the slot in case of priority conflict */
#define F_RESTORE       0x09

/* Global flags and variables */
#define F_REVERSE       0x40
#define F_BRAKE1        0x41
#define F_BRAKE2        0x42
#define F_BRAKE3        0x43
#define F_TRIGGER       0x44
#define F_SHIFT1        0x45
#define F_SHIFT2        0x46
#define F_SHIFT3        0x47
#define F_SHIFT4        0x48
#define F_SHIFT5        0x49
#define F_SHIFT6        0x4a
#define V_SPEED         0x4b
#define V_SPEED_REQUEST 0x4c
#define V_SPEED_CURRENT 0x4d
#define V_SELECT        0x4e /* Affects volume */
#define V_SHARE1        0x4f
#define V_SHARE2        0x50
#define V_SV_1          0x51
#define V_SV_2          0x52
#define V_SV_3          0x53
#define V_SV_4          0x54
#define V_SV_5          0x55
#define V_SV_6          0x56
#define V_SV_7          0x57
#define V_SV_8          0x58
#define V_SV_9          0x59
#define V_SV_10         0x5a
#define V_SV_11         0x5b
#define V_SV_12         0x5c
#define V_SV_13         0x5d
#define V_SV_14         0x5e
#define V_SV_15         0x5f
#define V_SV_16         0x60

/* Signed global variables */
#define V_ACCEL         0xE0

#endif
