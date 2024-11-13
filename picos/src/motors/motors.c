#include "motors.h"

#include "hardware/pwm.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"

#define PWM_TOP (999)
#define MAX_POWER_LEVEL (1000)
#define CLK_DIV (3.125f)

#define L_SLICE (2)
#define L_PWM_PIN (5)
#define L_PWM_CHAN (PWM_CHAN_B)
#define L_A_PIN (7)
#define L_B_PIN (6)

#define R_SLICE (1)
#define R_PWM_PIN (2)
#define R_PWM_CHAN (PWM_CHAN_A)
#define R_A_PIN (4)
#define R_B_PIN (3)

static void configure_motor(uint slice, uint pwm_pin, uint pwm_chan, uint a_pin, uint b_pin)
{
    gpio_set_function(pwm_pin, GPIO_FUNC_PWM);
    pwm_set_wrap(slice, PWM_TOP - 1);
    pwm_set_chan_level(slice, pwm_chan, 0);
    pwm_set_clkdiv(slice, CLK_DIV);
    pwm_set_phase_correct(slice, true);
    pwm_set_enabled(slice, true);

    gpio_init(a_pin);
    gpio_set_dir(a_pin, GPIO_OUT);
    gpio_put(a_pin, 1);

    gpio_init(b_pin);
    gpio_set_dir(b_pin, GPIO_OUT);
    gpio_put(b_pin, 1);
}

static void set_motor_pwm(uint slice, uint pwm_chan, uint a_pin, uint b_pin, int val)
{
    if (val > MAX_POWER_LEVEL) val = MAX_POWER_LEVEL;
    else if (val < -MAX_POWER_LEVEL) val = -MAX_POWER_LEVEL;
    if (val > 0)
    {
        pwm_set_chan_level(slice, pwm_chan, val);
        gpio_put(a_pin, 1);
        gpio_put(b_pin, 0);
    }
    else if (val < 0)
    {
        pwm_set_chan_level(slice, pwm_chan, -val);
        gpio_put(a_pin, 0);
        gpio_put(b_pin, 1);
    }
    else
    {
        pwm_set_chan_level(slice, pwm_chan, 0);
        gpio_put(a_pin, 1);
        gpio_put(b_pin, 1);
    }
}

void motors_init(void)
{
    configure_motor(L_SLICE, L_PWM_PIN, L_PWM_CHAN, L_A_PIN, L_B_PIN);
    configure_motor(R_SLICE, R_PWM_PIN, R_PWM_CHAN, R_A_PIN, R_B_PIN);
}

void motors_set(int left, int right)
{
    set_motor_pwm(L_SLICE, L_PWM_CHAN, L_A_PIN, L_B_PIN, left);
    set_motor_pwm(R_SLICE, R_PWM_CHAN, R_A_PIN, R_B_PIN, right);
}
