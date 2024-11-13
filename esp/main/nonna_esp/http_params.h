#ifndef _NONNA_ESP_HTTP_PARAMS_H_
#define _NONNA_ESP_HTTP_PARAMS_H_

#include <stdbool.h>
#include <stdint.h>

extern bool param_enabled;
extern double param_speed;
extern double param_kp;
extern double param_ki;
extern double param_kd;
extern int32_t param_mode;

void param_store_init(void);

void param_store_start(void);

#endif
