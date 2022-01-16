#pragma once
#ifndef FANCONTROLLER_APIFAN_H
#define FANCONTROLLER_APIFAN_H
#include "vector"
#include "Arduino.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "pico/stdio.h"
#include "Wire.h"


namespace APIFan {
class Fan {
private:
  u_short id{};
  float duty_cycle{};
  volatile u_short rpm{};
  u_short PWM_Control_Pin{};
  u_short RPM_Read_Pin{};
//  u_short MEASURE_TIME_MSEC{};
//  u_short MEASUREMENT_FREQ_TO_SECONDS_FACTOR{};
//  uint32_t start_rpm_measurement_time{};
  u_short MEASURE_TIME_MSEC = 1000;
  u_short MEASUREMENT_FREQ_TO_SECONDS_FACTOR = 1000 / MEASURE_TIME_MSEC;
  uint32_t start_rpm_measurement_time = 0;

public:
  uint slice_num = 255;
  u_short get_id();
  u_short get_rpm();
  float get_duty_cycle();
  void set_duty_cycle(float duty_cycle_percentage);
  void set_rpm(u_short new_rpm);
  u_short get_PWM_Control_Pin();
  u_short get_RPM_Read_Pin();
  void init_gpio();
  void measure_frequency();

  Fan(ushort PWM_Control_Pin, ushort PWM_RPM_Read_Pin, uint8_t id,
      float duty_cycle);
};
extern std::vector<Fan> FanArray;
void add_new_fan(Fan &fan_object);
void generate_pwm_cycle();
void update_rpm_all_fans();
void set_all_fans_to(float duty_cycle);
void read_all_fans();
void fans_init();
}
/* Never used
void generate_pwm_hardware(int gpio_pin, float pwm_percent);
static uint PWM_cycles;
float measure_duty_cycle(int gpio_pin);
*/

#endif // FANCONTROLLER_APIFAN_H
