#ifndef _NONNA_SENSORS_BRAIN_H_
#define _NONNA_SENSORS_BRAIN_H_

#include <stdint.h>

void brain_init(void);

void brain_decide_motors(uint32_t *pulse_lengths_us, int32_t *left_speed_out, int32_t *right_speed_out);

#endif
