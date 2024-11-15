#ifndef _NONNA_SENSORS_TURN_PID_H_
#define _NONNA_SENSORS_TURN_PID_H_

float turn_pid_update(float error);

void turn_pid_set_p(float p);

void turn_pid_set_d(float d);

#endif
