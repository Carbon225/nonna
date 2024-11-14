#include <stdio.h>

#include "pico/stdlib.h"
#include "pico/rand.h"
#include "hardware/watchdog.h"
#include "hardware/gpio.h"
#include "hardware/uart.h"

#include "pb_decode.h"

#include "nonna.pb.h"
#include "proto_framing.h"

#include "motors.h"

// #define DEBUG

#define UART_BAUD 115200

#define UART_SENSORS uart1
#define TX_SENSORS_PIN 8
#define RX_SENSORS_PIN 9

static int get_random_headers_to_skip(void)
{
    return 1 + get_rand_32() % 5;
}

static int handle_payload(const uint8_t *payload, size_t payload_len)
{
    pb_istream_t stream = pb_istream_from_buffer(payload, payload_len);
    nonna_proto_NonnaMsg msg;
    if (!pb_decode(&stream, nonna_proto_NonnaMsg_fields, &msg))
    {
        printf("Failed to decode message\n");
        return -1;
    }
    switch (msg.which_payload)
    {
    case nonna_proto_NonnaMsg_motor_cmd_tag:
#ifdef DEBUG
        printf(
            "Motor cmd:\n"
            "  left: %ld\n"
            "  right: %ld\n"
            "  idle: %d\n",
            msg.payload.motor_cmd.left,
            msg.payload.motor_cmd.right,
            msg.payload.motor_cmd.idle
        );
#endif
        if (msg.payload.motor_cmd.idle)
        {
            motors_set_idle();
        }
        else
        {
            motors_set_speed(msg.payload.motor_cmd.left, msg.payload.motor_cmd.right);
        }
        break;
    default:
        printf("Unknown message type: %d\n", msg.which_payload);
        return -1;
    }
    return 0;
}

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

    gpio_set_function(TX_SENSORS_PIN, GPIO_FUNC_UART);
    gpio_set_function(RX_SENSORS_PIN, GPIO_FUNC_UART);
    uart_init(UART_SENSORS, UART_BAUD);

    motors_init();

    uint8_t recv_buffer[MAX_PACKET_LEN];
    size_t recv_buffer_len = 0;
    int headers_to_skip = 0;

    for (;;)
    {
        // wait for header
        for (;;)
        {
            uint8_t recv_byte = (uint8_t) uart_getc(UART_SENSORS);
            if (recv_byte == FRAME_START)
            {
                if (headers_to_skip > 0)
                {
                    headers_to_skip--;
                }
                else
                {
                    recv_buffer[0] = recv_byte;
                    recv_buffer_len = 1;
                    break; // out of wait for header
                }
            }
        }

        uint8_t payload_len = (uint8_t) uart_getc(UART_SENSORS);
        if (payload_len > MAX_PAYLOAD_LEN)
        {
            printf("Payload too long: %u\n", payload_len);
            headers_to_skip = get_random_headers_to_skip();
            continue;
        }
        recv_buffer[1] = payload_len;
        recv_buffer_len = 2;

        // receive payload
        for (int i = 0; i < payload_len; i++)
        {
            recv_buffer[recv_buffer_len] = (uint8_t) uart_getc(UART_SENSORS);
            recv_buffer_len++;
        }

        // receive crc
        uint8_t crc = (uint8_t) uart_getc(UART_SENSORS);
        recv_buffer[recv_buffer_len] = crc;
        recv_buffer_len++;

        // parse frame
        const uint8_t *payload = unframe_bytes(recv_buffer, recv_buffer_len);
        if (payload == NULL)
        {
            printf("Invalid frame\n");
            headers_to_skip = get_random_headers_to_skip();
            continue;
        }

        if (handle_payload(payload, payload_len) != 0)
        {
            printf("Failed to handle payload\n");
            continue;
        }

        watchdog_update();
    }

    return 0;
}
