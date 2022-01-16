#include "APIFan.h"

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
#define HERTZ_TO_RPM_FACTOR 30.0

namespace APIFan {
// Start new cycle every n milliseconds
double PWM_Standard_Cycle_duration = 1000;

u_short Fan::get_id() { return this->id; }
u_short Fan::get_rpm() { return this->rpm; }

float Fan::get_duty_cycle() { return this->duty_cycle; }

void Fan::set_duty_cycle(float duty_cycle_percentage) {
  this->duty_cycle = duty_cycle_percentage;
}
void Fan::set_rpm(u_short new_rpm) { this->rpm = new_rpm; }
u_short Fan::get_PWM_Control_Pin() { return this->PWM_Control_Pin; }
u_short Fan::get_RPM_Read_Pin() { return this->RPM_Read_Pin; }

void Fan::init_gpio() {
  gpio_init(this->PWM_Control_Pin);
  gpio_set_dir(this->PWM_Control_Pin, GPIO_OUT);

  gpio_pull_up(this->RPM_Read_Pin);
  // Only the PWM B pins can be used as inputs.
  assert(pwm_gpio_to_channel(RPM_Read_Pin) == PWM_CHAN_B);
  this->slice_num = pwm_gpio_to_slice_num(RPM_Read_Pin);
  assert(this->slice_num != 255);
  pwm_config cfg = pwm_get_default_config();
  pwm_config_set_clkdiv_mode(&cfg, PWM_DIV_B_RISING);
  pwm_init(this->slice_num, &cfg, true);
  gpio_set_function(RPM_Read_Pin, GPIO_FUNC_PWM);
}

void Fan::measure_frequency() {
  this->slice_num = pwm_gpio_to_slice_num(RPM_Read_Pin); // STUPID CODE
  // todo: find out why we need to reassign slice number
  if (this->slice_num == 255)
    SERIAL_TO_USE.printf("Slice num is 255\r\n");
  uint32_t time_now = micros();
  if (time_now < this->start_rpm_measurement_time + 1000000) {
    //      SERIAL_TO_USE.printf("Now is less than waiting time for end of
    //      measurement\r\n");
    //      pwm_set_enabled(this->slice_num, true);
  } else {
    //      SERIAL_TO_USE.printf("We should take measurement for the fan
    //      %i\r\n",
    //                           this->id);
    // todo: implement timers instead of blocking sleep
    //    sleep_ms(MEASURE_TIME_MSEC);
    pwm_set_enabled(this->slice_num, false); // End measurement
    float input_signal_frequency = float(pwm_get_counter(this->slice_num)) *
                                   (float)MEASUREMENT_FREQ_TO_SECONDS_FACTOR;
    this->set_rpm(input_signal_frequency * (double)HERTZ_TO_RPM_FACTOR);
    this->start_rpm_measurement_time = time_now;
    pwm_set_counter(this->slice_num, 0);
    pwm_set_enabled(this->slice_num, true); // Start measurement
    //    SERIAL_TO_USE.printf("Fan %i: dc:%f RPM:%i\r\n", this->get_id(),
    //                         this->get_duty_cycle(), this->get_rpm());
  }
}

Fan::Fan(ushort PWM_Control_Pin, ushort PWM_RPM_Read_Pin, uint8_t id,
         float duty_cycle = 0) {
  this->PWM_Control_Pin = PWM_Control_Pin;
  this->RPM_Read_Pin = PWM_RPM_Read_Pin;
  this->set_duty_cycle(duty_cycle);
  this->id = id;
}
std::vector<Fan> FanArray = {};
 void add_new_fan(Fan &fan_object){
   FanArray.push_back(fan_object);
 }

void fans_init() {
  for (auto &fan : FanArray)
    fan.init_gpio();

  // If Serial is not initialized, initialize it
  if (!SERIAL_TO_USE)
    SERIAL_TO_USE.begin();
  SERIAL_TO_USE.println("FANApi GPIO init completed");
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
      // Check if output pin for fan1 should be high or low
      if (time_now <
          fan.get_duty_cycle() * 1000.0) { // 10 us * 100 steps = 1000
        gpio_put(fan.get_PWM_Control_Pin(), true);
      } else {
        gpio_put(fan.get_PWM_Control_Pin(), false);
      }
    }
  }
}

// Updates RPM values for all the fans
void update_rpm_all_fans() {
  for (auto &fan : FanArray) {
    fan.measure_frequency();
  }
}

void set_all_fans_to(float duty_cycle) {
  for (auto &fan : FanArray) {
    fan.set_duty_cycle(duty_cycle);
  }
}

void read_all_fans() {
  for (auto &fan : FanArray) {
    SERIAL_TO_USE.printf(
        "FAN #%i; on PWM:%2i RPM:%2i Duty cycle: %.2f; Slice# %d; RPM: %i \r\n",
        fan.get_id(), fan.get_PWM_Control_Pin(), fan.get_RPM_Read_Pin(),
        fan.get_duty_cycle(), fan.slice_num, fan.get_rpm());
  }
}
} // namespace APIFan

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

