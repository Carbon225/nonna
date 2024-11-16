#include "control_loop.h"

#include <stdbool.h>

#include "sensors.h"
#include "turn_pid.h"

#define CLAMP(x, min, max) ((x) < (min) ? (min) : ((x) > (max) ? (max) : (x)))
#define MOTOR_MAX_SPEED 1000

static void update_line_position(uint32_t *position, const uint32_t values[])
{
    uint32_t avg = 0;
    uint32_t sum = 0;
    bool onLine = false;

    for (int i = 0; i < SENSOR_ROW_SIZE; i++)
    {
        uint32_t v = values[i];

        if (v > SENSOR_BLACK_THRESHOLD_RAW)
        {
            onLine = true;
        }

        if (v > SENSOR_NOISE_THRESHOLD_RAW)
        {
            avg += i * v << 10;
            sum += v;
        }
    }

    if (!onLine)
    {
        // line position from 0 to 1024 * N (1024 per sensor)
        if (*position < (SENSOR_ROW_SIZE - 1) << 9)
        {
            *position = 0;
        }
        else
        {
            *position = (SENSOR_ROW_SIZE - 1) << 10;
        }
    }
    else
    {
        *position = avg / sum;
    }
}

void control_loop_init(void)
{

}

void control_loop_decide_motors(uint32_t *pulse_lengths_us, int32_t *left_speed_out, int32_t *right_speed_out)
{
    static uint32_t line_position = 0;
    static float forward_speed = 0.3f;

    float left_speed;
    float right_speed;

    update_line_position(&line_position, pulse_lengths_us);

    // 0 centered, 1.0 max left/right
    float error = line_position / ((SENSOR_ROW_SIZE - 1) * 512.f) - 1.f;
    bool off_line = error < -0.9f || error > 0.9f;

    float output = turn_pid_update(error);

    if (output > 0)
    {
        if (!off_line)
        {
            left_speed = forward_speed + output;
            right_speed = forward_speed - output;
        }
        else
        {
            left_speed = 0;
            right_speed = -output;
        }
    }
    else
    {
        if (!off_line)
        {
            left_speed = forward_speed + output;
            right_speed = forward_speed - output;
        }
        else
        {
            left_speed = output;
            right_speed = 0;
        }
    }

    *left_speed_out = CLAMP((int) (left_speed * MOTOR_MAX_SPEED), -MOTOR_MAX_SPEED, MOTOR_MAX_SPEED);
    *right_speed_out = CLAMP((int) (right_speed * MOTOR_MAX_SPEED), -MOTOR_MAX_SPEED, MOTOR_MAX_SPEED);
}
