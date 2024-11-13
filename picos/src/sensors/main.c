#include <stdio.h>

#include "pico/stdlib.h"
#include "hardware/watchdog.h"

#include "sensors.h"

// #define DEBUG

#define SENSOR_OVERSAMPLING 3
#define SENSOR_THRESHOLD 400

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

    sensors_init();

#ifndef DEBUG
    watchdog_disable();
    sleep_ms(10);
    watchdog_enable(100, 0);
#endif

    uint32_t pulse_lengths_us_1[APP_NUM_SENSORS] = {0};
    uint32_t pulse_lengths_us[APP_NUM_SENSORS] = {0};

    for (;;)
    {
        watchdog_update();

        for (int i = 0; i < APP_NUM_SENSORS; i++)
        {
            pulse_lengths_us[i] = 0;
        }
        for (int i = 0; i < SENSOR_OVERSAMPLING; i++)
        {
            sensors_read(pulse_lengths_us_1);
            for (int j = 0; j < APP_NUM_SENSORS; j++)
            {
                if (pulse_lengths_us_1[j] > pulse_lengths_us[j])
                {
                    pulse_lengths_us[j] = pulse_lengths_us_1[j];
                }
            }
        }

#ifdef DEBUG
        printf("Pulse lengths:\n");
        for (int i = 0; i < APP_NUM_SENSORS; i++)
        {
            printf(" %d: %lu us\n", i, pulse_lengths_us[i]);
        }
        printf("\n");
        sleep_ms(100);
#endif
    }

    return 0;
}
