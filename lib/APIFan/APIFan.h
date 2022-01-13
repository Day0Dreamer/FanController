#ifndef FANCONTROLLER_APIFAN_H
#define FANCONTROLLER_APIFAN_H

#include "Arduino.h"
#include "hardware/pwm.h"

void fan_setup();
void generate_pwm_hardware(int gpio_pin, float pwm_percent);
float measure_frequency(int gpio_pin);
float measure_duty_cycle(int gpio_pin);
void fan_routine();

static int frequency;
static int rpm;
static uint PWM_cycles;

static float fan_1_duty_cycle;

void set_pwm(uint8_t fan_id);

void generate_pwm_cycle();


#endif // FANCONTROLLER_APIFAN_H
