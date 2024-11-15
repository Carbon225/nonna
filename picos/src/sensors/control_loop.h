#ifndef _NONNA_SENSORS_CONTROL_LOOP_H_
#define _NONNA_SENSORS_CONTROL_LOOP_H_

#include <stdint.h>

void control_loop_init(void);

void control_loop_decide_motors(uint32_t *pulse_lengths_us, int32_t *left_speed, int32_t *right_speed);

#endif
