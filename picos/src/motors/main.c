#include <stdio.h>

#include "pico/stdlib.h"
#include "hardware/watchdog.h"

#include "motors.h"

// #define DEBUG

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

    motors_init();

#ifndef DEBUG
    watchdog_disable();
    sleep_ms(10);
    watchdog_enable(100, 0);
#endif

    for (;;)
    {
        watchdog_update();

        motors_set(300, 300);

#ifdef DEBUG
        sleep_ms(100);
#endif
    }

    return 0;
}
