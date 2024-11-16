#include "brain.h"

#include <stdio.h>

#include "neural_network.h"
#include "sensors.h"

#define MOTOR_SCALING 300

#define CLAMP(x, min, max) ((x) < (min) ? (min) : ((x) > (max) ? (max) : (x)))
#define MOTOR_MAX_SPEED 1000

static float inputs[26] = {0};
static float outputs[4] = {0};

void brain_init(void)
{

}

void brain_decide_motors(uint32_t *sensors, int32_t *motor_left, int32_t *motor_right)
{
    // convert to floats
    for (int i = 0; i < APP_NUM_SENSORS; i++)
    {
        inputs[i] = sensors[i] > SENSOR_BLACK_THRESHOLD_RAW ? 1.0f : 0.0f;
    }

    // compute
    forward(inputs, outputs);

    // printf("Network out: %f\t%f\n", outputs[0], outputs[1]);

    // store memory
    inputs[24] = outputs[2];
    inputs[25] = outputs[3];

    // control motors
    *motor_left = CLAMP(outputs[0] * MOTOR_SCALING, -MOTOR_MAX_SPEED, MOTOR_MAX_SPEED);
    *motor_right = CLAMP(outputs[1] * MOTOR_SCALING, -MOTOR_MAX_SPEED, MOTOR_MAX_SPEED);
}
