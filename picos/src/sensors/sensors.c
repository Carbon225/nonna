#include "sensors.h"

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/sync.h"

#define SENSOR_SETTLE_DELAY_US (150)
#define SENSOR_TIMEOUT_US (500)

#define LED_ENABLE_PIN (2)

static const uint SENSOR_PINS[APP_NUM_SENSORS] = {
    27, 26, 21, 20, 15, 14, 9, 8,
    28, 25, 22, 19, 16, 13, 10, 7,
    29, 24, 23, 18, 17, 12, 11, 6,
};

static uint32_t SENSOR_GPIO_MASK = 0;

void sensors_init(void)
{
    gpio_init(LED_ENABLE_PIN);
    gpio_set_dir(LED_ENABLE_PIN, GPIO_OUT);
    gpio_put(LED_ENABLE_PIN, 1);

    for (int i = 0; i < APP_NUM_SENSORS; i++)
    {
        gpio_init(SENSOR_PINS[i]);
        gpio_set_dir(SENSOR_PINS[i], GPIO_OUT);
        gpio_put(SENSOR_PINS[i], 1);
        gpio_disable_pulls(SENSOR_PINS[i]);
        SENSOR_GPIO_MASK |= (1 << SENSOR_PINS[i]);
    }
}

void sensors_read(uint32_t *pulse_lengths_us)
{
    gpio_put(LED_ENABLE_PIN, 0);

    for (int i = 0; i < APP_NUM_SENSORS; i++)
    {
        pulse_lengths_us[i] = UINT32_MAX;
    }

    gpio_set_dir_out_masked(SENSOR_GPIO_MASK);
    busy_wait_us(SENSOR_SETTLE_DELAY_US);

    uint32_t irq_state = save_and_disable_interrupts();

    int n_readings = 0;
    uint32_t now;
    uint32_t read_start_us = time_us_32();
    gpio_set_dir_in_masked(SENSOR_GPIO_MASK);

    do
    {
        now = time_us_32();
        uint32_t gpios = gpio_get_all();

        for (int i = 0; i < APP_NUM_SENSORS; i++)
        {
            if (pulse_lengths_us[i] != UINT32_MAX)
            {
                continue;
            }

            uint32_t mask = (1 << SENSOR_PINS[i]);
            uint32_t pin_state = (gpios & mask);

            if (pin_state == 0)
            {
                pulse_lengths_us[i] = now - read_start_us;
                n_readings++;
            }
        }
    }
    while (n_readings < APP_NUM_SENSORS && now - read_start_us < SENSOR_TIMEOUT_US);

    restore_interrupts(irq_state);

    gpio_put(LED_ENABLE_PIN, 1);

    for (int i = 0; i < APP_NUM_SENSORS; i++)
    {
        if (pulse_lengths_us[i] == UINT32_MAX)
        {
            pulse_lengths_us[i] = now - read_start_us;
        }
    }
}

void sensors_read_oversampled(uint32_t *pulse_lengths_us, int oversampling)
{
    uint32_t pulse_lengths_us_1[APP_NUM_SENSORS];
    for (int i = 0; i < APP_NUM_SENSORS; i++)
    {
        pulse_lengths_us[i] = 0;
    }
    for (int s = 0; s < oversampling; s++)
    {
        sensors_read(pulse_lengths_us_1);
        for (int i = 0; i < APP_NUM_SENSORS; i++)
        {
            if (pulse_lengths_us_1[i] > pulse_lengths_us[i])
            {
                pulse_lengths_us[i] = pulse_lengths_us_1[i];
            }
        }
    }
}
