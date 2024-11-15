#include <stdio.h>

#include "pico/stdlib.h"
#include "hardware/watchdog.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"

#include "pb_encode.h"
#include "proto_framing.h"
#include "nonna.pb.h"

#include "sensors.h"
#include "control_loop.h"

// #define DEBUG

#define SENSOR_OVERSAMPLING 1

#define AUTOMATIC_ARMING 1
#define AUTOMATIC_ARMING_DELAY_US 3000000

#define AUTOMATIC_DISARMING 1

#define UART_BAUD 115200

#define UART_MOTORS uart1
#define TX_MOTORS_PIN 4
#define RX_MOTORS_PIN 5

int main()
{
#ifndef DEBUG
    watchdog_enable(4000, 0);
#endif

    stdio_init_all();

#ifdef DEBUG
    while (!stdio_usb_connected())
    {
        sleep_ms(10);
    }
#endif

    if (watchdog_caused_reboot())
    {
        printf("Watchdog caused reboot\n");
    }

    gpio_set_function(TX_MOTORS_PIN, GPIO_FUNC_UART);
    gpio_set_function(RX_MOTORS_PIN, GPIO_FUNC_UART);
    uart_init(UART_MOTORS, UART_BAUD);

    sensors_init();
    control_loop_init();

    uint32_t pulse_lengths_us[APP_NUM_SENSORS];
    bool enabled = false;

#if AUTOMATIC_ARMING
    uint32_t first_time_line_detected_us = 0;
#endif

    for (;;)
    {
#if SENSOR_OVERSAMPLING > 1
        // sensors_read_oversampled(pulse_lengths_us, SENSOR_OVERSAMPLING);
#else
        sensors_read(pulse_lengths_us);
#endif

#ifdef DEBUG
        printf("Pulse lengths:\n");
        for (int i = 0; i < APP_NUM_SENSORS; i++)
        {
            printf(" %d: %lu us\n", i, pulse_lengths_us[i]);
        }
        printf("\n");
        sleep_ms(100);
#endif

#if AUTOMATIC_DISARMING
        if (enabled)
        {
            bool all_sensors_infinite = true;
            for (int i = 0; i < APP_NUM_SENSORS; i++)
            {
                if (pulse_lengths_us[i] < SENSOR_BLACK_THRESHOLD)
                {
                    all_sensors_infinite = false;
                    break;
                }
            }
            if (all_sensors_infinite)
            {
                printf("Automatic disarming\n");
                enabled = false;
            }
        }
#endif

#if AUTOMATIC_ARMING
        if (!enabled)
        {
            bool edge_sensors_white =
                pulse_lengths_us[0] < SENSOR_BLACK_THRESHOLD &&
                pulse_lengths_us[7] < SENSOR_BLACK_THRESHOLD &&
                pulse_lengths_us[8] < SENSOR_BLACK_THRESHOLD &&
                pulse_lengths_us[15] < SENSOR_BLACK_THRESHOLD &&
                pulse_lengths_us[16] < SENSOR_BLACK_THRESHOLD &&
                pulse_lengths_us[23] < SENSOR_BLACK_THRESHOLD;
            bool middle_sensors_black = false;
            for (int row = 0; row < 3; row++)
            {
                for (int col = 1; col < 7; col++)
                {
                    if (pulse_lengths_us[row * 8 + col] > SENSOR_BLACK_THRESHOLD)
                    {
                        middle_sensors_black = true;
                        break;
                    }
                }
            }
            if (edge_sensors_white && middle_sensors_black)
            {
                if (first_time_line_detected_us == 0)
                {
                    first_time_line_detected_us = time_us_32();
                }
                else if (time_us_32() - first_time_line_detected_us > AUTOMATIC_ARMING_DELAY_US)
                {
                    printf("Automatic arming\n");
                    enabled = true;
                }
            }
            else
            {
                first_time_line_detected_us = 0;
            }
        }
#endif

        nonna_proto_NonnaMsg msg = {0};
        msg.which_payload = nonna_proto_NonnaMsg_motor_cmd_tag;
        msg.payload.motor_cmd.idle = !enabled;

        control_loop_decide_motors(pulse_lengths_us, &msg.payload.motor_cmd.left, &msg.payload.motor_cmd.right);

        uint8_t payload_buf[MAX_PAYLOAD_LEN];
        pb_ostream_t pb_ostream = pb_ostream_from_buffer(payload_buf, MAX_PAYLOAD_LEN);
        if (!pb_encode(&pb_ostream, nonna_proto_NonnaMsg_fields, &msg))
        {
            printf("Failed to encode message\n");
            continue;
        }

        uint8_t packet_buf[MAX_PACKET_LEN];
        int packet_size = frame_bytes(payload_buf, pb_ostream.bytes_written, packet_buf, MAX_PACKET_LEN);
        if (packet_size < 0)
        {
            printf("Failed to frame message\n");
            continue;
        }

        uart_write_blocking(UART_MOTORS, packet_buf, packet_size);

        watchdog_update();
    }

    return 0;
}
