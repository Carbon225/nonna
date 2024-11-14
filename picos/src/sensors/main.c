#include <stdio.h>

#include "pico/stdlib.h"
#include "hardware/watchdog.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"

#include "pb_encode.h"
#include "proto_framing.h"
#include "nonna.pb.h"
#include "bootsel_button.h"

#include "sensors.h"

// #define DEBUG

#define SENSOR_OVERSAMPLING 3
#define SENSOR_THRESHOLD 400

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

    uint32_t pulse_lengths_us[APP_NUM_SENSORS] = {0};

    bool enabled = false;

    for (;;)
    {
        sensors_read_oversampled(pulse_lengths_us, SENSOR_OVERSAMPLING);

#ifdef DEBUG
        printf("Pulse lengths:\n");
        for (int i = 0; i < APP_NUM_SENSORS; i++)
        {
            printf(" %d: %lu us\n", i, pulse_lengths_us[i]);
        }
        printf("\n");
        sleep_ms(100);
#endif

        if (bootsel_button_get())
        {
            enabled = !enabled;
            for (int i = 0; i < 100; i++)
            {
                sleep_ms(10);
                watchdog_update();
            }
            printf("Enabled: %d\n", enabled);
        }

        nonna_proto_NonnaMsg msg = {0};
        msg.which_payload = nonna_proto_NonnaMsg_motor_cmd_tag;
        msg.payload.motor_cmd.left = enabled ? 300 : 0;
        msg.payload.motor_cmd.right = enabled ? 300 : 0;
        msg.payload.motor_cmd.idle = !enabled;

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
