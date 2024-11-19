#include "control_loop.h"

#include <stdbool.h>

#include "sensors.h"
#include "turn_pid.h"

#define CLAMP(x, min, max) ((x) < (min) ? (min) : ((x) > (max) ? (max) : (x)))
#define MOTOR_MAX_SPEED 1000

static void extract_sensor_row(uint32_t *values, uint32_t *row)
{
    row[7] = values[23];
    row[6] = values[14];
    row[5] = values[13];
    row[4] = values[12];
    row[3] = values[11];
    row[2] = values[10];
    row[1] = values[9];
    row[0] = values[16];
}

static void update_line_position(uint32_t *position, const uint32_t values[])
{
    uint32_t avg = 0;
    uint32_t sum = 0;
    bool onLine = false;

    for (int i = 0; i < SENSOR_ROW_SIZE; i++)
    {
        uint32_t v = values[i];

        if (v > SENSOR_BLACK_THRESHOLD_CALIBRATED)
        {
            onLine = true;
        }

        if (v > SENSOR_NOISE_THRESHOLD_CALIBRATED)
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
    static const float forward_speed = 0.20f;
    static const float sharp_turn_speed = 2.0f * forward_speed;

    static uint32_t line_position = 0;

    float left_speed;
    float right_speed;
    uint32_t sensor_row[SENSOR_ROW_SIZE];

    bool middle_detected = pulse_lengths_us[3] > SENSOR_BLACK_THRESHOLD_CALIBRATED || pulse_lengths_us[4] > SENSOR_BLACK_THRESHOLD_CALIBRATED;
    bool left_turn_detected = (!middle_detected) && pulse_lengths_us[7] > SENSOR_BLACK_THRESHOLD_CALIBRATED;
    bool right_turn_detected = (!middle_detected) && pulse_lengths_us[0] > SENSOR_BLACK_THRESHOLD_CALIBRATED;

    extract_sensor_row(pulse_lengths_us, sensor_row);
    update_line_position(&line_position, sensor_row);

    // 0 centered, 1.0 max left/right
    float error = line_position / ((SENSOR_ROW_SIZE - 1) * 512.f) - 1.f;
    bool off_line = error < -0.9f || error > 0.9f;

    float output = turn_pid_update(error);

    if (left_turn_detected || (off_line && output < 0))
    {
        left_speed = forward_speed - sharp_turn_speed;
        right_speed = forward_speed + sharp_turn_speed;
    }
    else if (right_turn_detected || (off_line && output > 0))
    {
        left_speed = forward_speed + sharp_turn_speed;
        right_speed = forward_speed - sharp_turn_speed;
    }
    else
    {
        left_speed = forward_speed + output;
        right_speed = forward_speed - output;
    }

    *left_speed_out = CLAMP((int) (left_speed * MOTOR_MAX_SPEED), -MOTOR_MAX_SPEED, MOTOR_MAX_SPEED);
    *right_speed_out = CLAMP((int) (right_speed * MOTOR_MAX_SPEED), -MOTOR_MAX_SPEED, MOTOR_MAX_SPEED);
}
