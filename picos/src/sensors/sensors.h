#ifndef _NONNA_SENSORS_H_
#define _NONNA_SENSORS_H_

#include <stdint.h>

#define APP_NUM_SENSORS 24

void sensors_init(void);

void sensors_read(uint32_t *pulse_lengths_us);

#endif
