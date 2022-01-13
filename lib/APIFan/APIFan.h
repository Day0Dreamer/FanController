#ifndef FANCONTROLLER_APIFAN_H
#define FANCONTROLLER_APIFAN_H

#include "Arduino.h"
#include "hardware/pwm.h"
#include "vector"

void fan_setup();
float measure_frequency(int gpio_pin);
void generate_pwm_cycle();
void update_rpm_all_fans();
//class Fan;
//std::vector <Fan> FanArray;
void set_all_fans_to(float duty_cycle);
void read_all_fans();

/* Never used
void generate_pwm_hardware(int gpio_pin, float pwm_percent);
static uint PWM_cycles;
float measure_duty_cycle(int gpio_pin);
*/

#endif // FANCONTROLLER_APIFAN_H
