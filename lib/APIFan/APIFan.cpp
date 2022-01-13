#include "Arduino.h"
#include "hardware/pwm.h"

#ifdef NO_USB
/* hardware UART*/
#define SERIAL_TO_USE Serial1
#else
/* USB */
#define SERIAL_TO_USE Serial
#endif

// One full rotation gives off 2 Hertz
// 1 RPS (60 RPM) means 2 Hertz
// Therefore:
// (60 RPM/RPS) / (2 Hertz/RPS) = 30 Hz/RPM
// RPM = Hz * 30
#define HERTZ_TO_RPM_FACTOR 30

#include "Arduino.h"
#include "SPI.h"
#include "Wire.h"
#include "hardware/gpio.h"
#include "pico/stdio.h"
#include "pico/stdlib.h"
#include "vector"

// Start new cycle every n milliseconds
double PWM_Standard_Cycle_duration = 1000;

class Fan {
private:
  u_short id{};
  float duty_cycle{};
  u_short rpm{};
  u_short PWM_Control_Pin{};
  u_short RPM_Read_Pin{};

public:
  u_short get_id() { return this->id; }
  u_short get_rpm() { return this->rpm; }
  float get_duty_cycle() { return this->duty_cycle; }
  void set_duty_cycle(float duty_cycle_percentage) {
    this->duty_cycle = duty_cycle_percentage;
  }
  void set_rpm(uint new_rpm) { this->rpm = new_rpm; }
  u_short get_PWM_Control_Pin() { return this->PWM_Control_Pin; }
  u_short get_RPM_Read_Pin() { return this->RPM_Read_Pin; }

  void init_gpio() {
    gpio_init(this->PWM_Control_Pin);
    gpio_set_dir(this->PWM_Control_Pin, GPIO_OUT);

    gpio_pull_up(this->RPM_Read_Pin);
  }

  Fan(ushort PWM_Control_Pin, ushort PWM_RPM_Read_Pin, uint8_t id,
      float duty_cycle = 0) {
    this->PWM_Control_Pin = PWM_Control_Pin;
    this->RPM_Read_Pin = PWM_RPM_Read_Pin;
    this->set_duty_cycle(duty_cycle);
    this->id = id;
  }
};

Fan fan_1(18, 19, 1, 0.075);
Fan fan_2(26, 27, 2, 0.005);
Fan fan_3(6, 7, 3, 0.075);
Fan fan_4(8, 9, 4, 0.075);
Fan fan_5(10, 11, 5, 0.075);
Fan fan_6(12, 13, 6, 0.075);
Fan fan_7(14, 15, 7, 0.075);
Fan fan_8(16, 17, 8, 0.075);

std::vector FanArray = {fan_1, fan_2, fan_3, fan_4, fan_5, fan_6, fan_7, fan_8};

void fan_setup() {
  fan_1.init_gpio();
  fan_2.init_gpio();
  fan_3.init_gpio();
  fan_4.init_gpio();
  fan_5.init_gpio();
  //  fan_6.init_gpio();
  fan_7.init_gpio();
  fan_8.init_gpio();
  // If Serial is not initialized, initialize it
  if (!SERIAL_TO_USE)
    SERIAL_TO_USE.begin();
  SERIAL_TO_USE.println("FANApi GPIO init completed");
}
#define MEASURE_TIME_MSEC 1000
#define MEASUREMENT_FREQ_TO_SECONDS_FACTOR 1000 / MEASURE_TIME_MSEC
float measure_frequency(int gpio_pin) {
  // Only the PWM B pins can be used as inputs.
  assert(pwm_gpio_to_channel(gpio_pin) == PWM_CHAN_B);
  uint slice_num = pwm_gpio_to_slice_num(gpio_pin);
  pwm_config cfg = pwm_get_default_config();
  pwm_config_set_clkdiv_mode(&cfg, PWM_DIV_B_RISING);
  pwm_init(slice_num, &cfg, false);
  gpio_set_function(gpio_pin, GPIO_FUNC_PWM);

  pwm_set_enabled(slice_num, true);
  // todo: implement timers instead of blocking sleep
  sleep_ms(MEASURE_TIME_MSEC);
  pwm_set_enabled(slice_num, false);
  float input_signal_frequency =
      float(pwm_get_counter(slice_num)) * MEASUREMENT_FREQ_TO_SECONDS_FACTOR;
  return input_signal_frequency;
}

// Call this function with a frequency of 100kHz (every 10 microseconds) so it
// will walk through all the fans and check if any pins should be set HIGH or
// LOW. It is aimed to check 100 times for inside one 1kHz cycle for a proper
// time to switch the pin.
void generate_pwm_cycle() {
  // this is called every 10 microseconds (0.01ms)
  for (uint8_t t = 0; t < 100; ++t) {
    // read the time (only last 3 digits)
    ulong time_now = micros() % 1000;

    for (auto &fan : FanArray) {
//      SERIAL_TO_USE.printf("Generating PWM signal for FAN #%d", fan);
      // Check if output pin for fan1 should be high or low
      if (time_now < fan.get_duty_cycle() * 1000.0) { // 10 us * 100 steps = 1000
        gpio_put(fan.get_PWM_Control_Pin(), true);
//        SERIAL_TO_USE.printf("time now is %d\r\n", time_now);
//        SERIAL_TO_USE.printf("fan.get_duty_cycle() is %f\r\n", fan.get_duty_cycle());
//        SERIAL_TO_USE.printf("(double)fan.get_duty_cycle() is %f\r\n", (double)fan.get_duty_cycle());
//        SERIAL_TO_USE.printf("(double)fan.get_duty_cycle() * 1000UL is %f\r\n", (double)fan.get_duty_cycle() * 1000UL);
//        SERIAL_TO_USE.printf("FAN #%d is pulled HIGH\r\n", fan);
      } else {
        gpio_put(fan.get_PWM_Control_Pin(), false);
//        SERIAL_TO_USE.printf("FAN #%d is pulled LOW\r\n", fan);
      }
    }
  }
}

// Updates RPM values for all the fans
void update_rpm_all_fans() {
  for (auto &fan : FanArray) {
    uint frequency = uint(measure_frequency(fan.get_RPM_Read_Pin()));
    fan.set_rpm(frequency * HERTZ_TO_RPM_FACTOR);
    SERIAL_TO_USE.printf("Fan %i: dc:%f RPM:%i\r\n", fan.get_id(),
                         fan.get_duty_cycle(), fan.get_rpm());
  }
}

void set_all_fans_to(float duty_cycle) {
  for (auto &fan : FanArray) {
    fan.set_duty_cycle(duty_cycle);
  }
}

void read_all_fans() {
  for (auto &fan : FanArray) {
    SERIAL_TO_USE.printf("FAN #%i; Duty cycle: %.2f; RPM: %i \r\n",fan.get_id(), fan.get_duty_cycle(), fan.get_rpm());
  }
}

/* Cool, but unused code

uint PWM_cycles = 500; // How many CPU cycles shall pass for 1 PWM tick
// Enable PWM generation with low 1KHz frequency and defined duty cycle
void generate_pwm_hardware(int gpio_pin, float pwm_percent) {
  // PIN is considered initiated if it is set up as PWM
  bool initiated = (gpio_get_function(gpio_pin) == GPIO_FUNC_PWM);
  // For the first call we set up certain functions
  if (!initiated) {
    //        pin_init(gpio_pin);
    gpio_set_function(gpio_pin, GPIO_FUNC_PWM);
    SERIAL_TO_USE.printf("Pin %d has been set to %f percent\n", gpio_pin,
                         pwm_percent);
    uint square_signal_pin_slice = pwm_gpio_to_slice_num(gpio_pin);
    SERIAL_TO_USE.printf("Pin %d belongs to slice # %d\n", gpio_pin,
                         square_signal_pin_slice);
    // For every n CPU cycles it appears like 1 CPU cycle has passed
    pwm_set_clkdiv(square_signal_pin_slice, 256); // Max divider of 256
    // Reset PWM wave after n pwm cycles
    pwm_set_wrap(square_signal_pin_slice, PWM_cycles - 1);
  }

  // We are supposed to be initiated by this point
  uint square_signal_pin_slice = pwm_gpio_to_slice_num(gpio_pin);
  pwm_set_chan_level(square_signal_pin_slice, PWM_CHAN_A,
                     int(PWM_cycles * pwm_percent));
  pwm_set_chan_level(square_signal_pin_slice, PWM_CHAN_B,
                     int(PWM_cycles - (PWM_cycles * pwm_percent)));
  SERIAL_TO_USE.printf("Pin %d is set to %d on %d off out of %d ticks\n",
                       gpio_pin, int(PWM_cycles * pwm_percent),
                       int(PWM_cycles - (PWM_cycles * pwm_percent)),
                       PWM_cycles);
  pwm_set_enabled(square_signal_pin_slice, true);
}

// Cool thing that measures duty cycles, never used though
float measure_duty_cycle(int gpio_pin) {
  // Only the PWM B pins can be used as inputs.
  assert(pwm_gpio_to_channel(gpio_pin) == PWM_CHAN_B);
  uint slice_num = pwm_gpio_to_slice_num(gpio_pin);

  // Count once for every 100 cycles the PWM B input is high
  pwm_config cfg = pwm_get_default_config();
  pwm_config_set_clkdiv_mode(&cfg, PWM_DIV_B_HIGH);
  pwm_config_set_clkdiv(&cfg, 100);
  pwm_init(slice_num, &cfg, false);
  gpio_set_function(gpio_pin, GPIO_FUNC_PWM);

  pwm_set_enabled(slice_num, true);
  sleep_ms(10);
  pwm_set_enabled(slice_num, false);
  float counting_rate = clock_get_hz(clk_sys) / 100;
  float max_possible_count = counting_rate * 0.01;
  return pwm_get_counter(slice_num) / max_possible_count;
}


*/