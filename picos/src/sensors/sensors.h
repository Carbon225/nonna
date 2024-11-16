#ifndef _NONNA_SENSORS_H_
#define _NONNA_SENSORS_H_

#include <stdint.h>

#define APP_NUM_SENSORS 24
#define SENSOR_ROW_SIZE 8
#define SENSOR_COL_SIZE 3
#define SENSOR_BLACK_THRESHOLD_RAW 150
#define SENSOR_BLACK_THRESHOLD_CALIBRATED 512
#define SENSOR_NOISE_THRESHOLD_RAW 0
#define SENSOR_NOISE_THRESHOLD_CALIBRATED 0

void sensors_init(void);

void sensors_read(uint32_t *pulse_lengths_us);

void sensors_read_oversampled(uint32_t *pulse_lengths_us, int oversampling);

void sensors_calibrate(void);

void sensors_apply_calibration(uint32_t *values_in, uint32_t *values_out);

#endif
