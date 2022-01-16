#include <APICan.h>
#include <APIFan.h>

// todo: Clean up the code
// todo: Make PWM Generation start by alarm
// todo: Make RPM readout start by alarm

#define NO_USB
#ifdef NO_USB
/* hardware UART*/
#define SERIAL_TO_USE Serial1
#else
/* USB */
#define SERIAL_TO_USE Serial
#endif

#ifndef ARDUINO_ARCH_RP2040
#error "Select a Raspberry Pi Pico board"
#endif

#define Fan_1_RPM_Status_report 0b00000000000000100000000000000
#define Fan_1_Duty_Cycle_Status_report 0b00000000000000100000000000001
#define Fan_1_OnOff_Status_report 0b00000000000000100000000000010
#define Fan_1_Cooler_failure_code_Status_report 0b00000000000000100000000000011
#define Fan_1_Duty_Cycle_CMD_Set_Duty_Cycle 0b00000000000000000000000000100
#define Fan_1_OnOff_CMD_Set_on_or_off_state 0b00000000000000000000000000101
#define Fan_1_Duty_Cycle_ACK_Set_Duty_Cycle 0b00000000000000010000000000110
#define Fan_1_OnOff_ACK_Set_on_or_off_state 0b00000000000000010000000000111
#define Fan_2_RPM_Status_report 0b00000000000001000000000000000
#define Fan_3_RPM_Status_report 0b00000000000001100000000000000

// PIN DEFINITIONS
// SPI
#define INT_PIN 255 // INT output of MCP2515 (adapt to your design)
#define SCK_PIN 2   // SCK input of MCP2515
#define MOSI_PIN 3  // SDI input of MCP2515
#define MISO_PIN 4  // SDO output of MCP2517
#define CS_PIN 5    // CS input of MCP2515 (adapt to your design)

void setup();
void loop();
void fan_setup();
bool repeating_timer_callback(struct repeating_timer *t);
int read_input(float &temp_duty_cycle);

// Global objects initialization
APICan can_object(CS_PIN, INT_PIN, 16000000UL, &SERIAL_TO_USE);
uint32_t time_start;
uint32_t time_end;

extern "C" {
#include "pico/stdio_uart.h"
#include "pico/stdlib.h"
}
[[noreturn]] void repeat_back_pico_sdk() {
  stdio_uart_init_full(uart0, 115200, 0, 1);
  char buffer;

  while (true) {
    if (uart_is_readable(uart0)) {
      buffer = getchar();
      printf("%c", buffer);
      if (buffer == '\r' || buffer == '\n') {
        printf("\n\r");
      }
    }
    printf("\r\n");
  }
}
bool repeating_timer_callback(struct repeating_timer *t) {
  printf("Repeat at %lld\n", time_us_64());
  return true;
}

void fan_setup() {
  APIFan::Fan fan_0(16, 17, 0, 0.075);
  APIFan::add_new_fan(fan_0);
  APIFan::Fan fan_1(18, 19, 1, 0.075);
  APIFan::add_new_fan(fan_1);
  APIFan::Fan fan_2(20, 21, 2, 0.075);
  APIFan::add_new_fan(fan_2);
  APIFan::Fan fan_3(6, 7, 3, 0.075);
  APIFan::add_new_fan(fan_3);
  APIFan::Fan fan_4(8, 9, 4, 0.075);
  APIFan::add_new_fan(fan_4);
  APIFan::Fan fan_5(10, 11, 5, 0.005);
  APIFan::add_new_fan(fan_5);
  APIFan::Fan fan_6(12, 13, 6, 0.075);
  APIFan::add_new_fan(fan_6);
  APIFan::Fan fan_7(14, 15, 7, 0.075);
  APIFan::add_new_fan(fan_7);
}

uint8_t user_input_buffer;
float temp_duty_cycle_main = .75;
// Reads user input over console and reacts to it
int read_input(float &temp_duty_cycle) {
  if (SERIAL_TO_USE.available()) {
    user_input_buffer = SERIAL_TO_USE.read();
    if (user_input_buffer == '`') {
      temp_duty_cycle = 0.0;
      APIFan::set_all_fans_to(temp_duty_cycle);
      SERIAL_TO_USE.printf("All fans has been set to %.2f\r\n",
                           temp_duty_cycle);
      return 0;
    }
    if (user_input_buffer == '0') {
      temp_duty_cycle = 1.0;
      APIFan::set_all_fans_to(temp_duty_cycle);
      SERIAL_TO_USE.printf("All fans has been set to %.2f\r\n",
                           temp_duty_cycle);
      return 0;
    } else if (user_input_buffer == '5') {
      temp_duty_cycle = 0.5;
      APIFan::set_all_fans_to(temp_duty_cycle);
      SERIAL_TO_USE.printf("All fans has been set to %.2f\r\n",
                           temp_duty_cycle);
      return 0;
    } else if (user_input_buffer == '1') {
      temp_duty_cycle = 0.1;
      APIFan::set_all_fans_to(temp_duty_cycle);
      SERIAL_TO_USE.printf("All fans has been set to %.2f\r\n",
                           temp_duty_cycle);
      return 0;
    } else if (user_input_buffer == '-') {
      temp_duty_cycle -= 0.01;
      APIFan::set_all_fans_to(temp_duty_cycle);
      SERIAL_TO_USE.printf("All fans has been set to %.2f\r\n",
                           temp_duty_cycle);
      return 0;
    } else if (user_input_buffer == '=') {
      temp_duty_cycle += 0.01;
      APIFan::set_all_fans_to(temp_duty_cycle);
      SERIAL_TO_USE.printf("All fans has been set to %.2f\r\n",
                           temp_duty_cycle);
      return 0;
    } else if (user_input_buffer == 'f') {
      SERIAL_TO_USE.printf("═══════════════════════════════════════════════════"
                           "═══════════════════\r\n");
      APIFan::read_all_fans();
      SERIAL_TO_USE.printf("═══════════════════════════════════════════════════"
                           "═══════════════════\r\n");
      return 0;
    }
    if (user_input_buffer == '\r' || user_input_buffer == '\n') {
      SERIAL_TO_USE.print("\n\r");
    }
  }
  return 0;
}

void setup() {
  // Init SPI Bus
  APICan::setSPI(SCK_PIN, MOSI_PIN, MISO_PIN, CS_PIN);

  // Init Serial
  SERIAL_TO_USE.setTX(0);
  SERIAL_TO_USE.setRX(1);
  // Start serial
  SERIAL_TO_USE.begin(115200);
  SERIAL_TO_USE.println("Serial initiated");

  // Init blinking LED
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  SERIAL_TO_USE.println("LED initiated");
#ifdef WAIT_FOR_SERIAL
  // Wait for serial (blink led at 10 Hz during waiting)
  while (!SERIAL_TO_USE) {
    delay(50);
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  }
#endif

  // Init CAN2515 module
  can_object.init();
  can_object.configure_chip();
  SERIAL_TO_USE.println("CAN initiated");

  // Init APIFan
  fan_setup();
  APIFan::fans_init();
  SERIAL_TO_USE.println("FAN Api initiated");

  // Arm the timers
  //  struct repeating_timer timer;
  //  add_repeating_timer_ms(5000, repeating_timer_callback, nullptr, &timer);
  //  SERIAL_TO_USE.println("Added repeating timer");
}

void loop() {
  //  SERIAL_TO_USE.printf("═══════════════════════════════════════════════════════"
  //                       "═══════════════\r\n");

  //  time_start = micros();
  read_input(temp_duty_cycle_main);
  //  time_end = micros();
  //  SERIAL_TO_USE.printf("read_input took %d microseconds\r\n",
  //                       time_end - time_start);

  //  time_start = micros();
  APIFan::generate_pwm_cycle();
  //  time_end = micros();
  //  SERIAL_TO_USE.printf("generate_pwm_cycle took %d microseconds\r\n",
  //                       time_end - time_start);

  //  time_start = micros();
  APIFan::update_rpm_all_fans();
  //  time_end = micros();
  //  SERIAL_TO_USE.printf("update_rpm_all_fans took %d microseconds\r\n",
  //                       time_end - time_start);

  //  SERIAL_TO_USE.printf("═══════════════════════════════════════════════════════"
  //                       "═══════════════\r\n");
  //    int64_t message = millis();
  if (millis() % 100 == 0) {
    SERIAL_TO_USE.printf("New batch of sending out triggered\r\n",
                         APIFan::FanArray[0].get_rpm());
    for (auto &fan : APIFan::FanArray) {
      //    SERIAL_TO_USE.printf("Sending Fan_1_RPM_Status_report: %i\t\t",
      //    APIFan::FanArray[0].get_rpm());
      can_object.send_message(Fan_1_RPM_Status_report, fan.get_rpm());
      can_object.send_message(Fan_1_Duty_Cycle_Status_report, fan.get_duty_cycle()*100);
      can_object.send_message(Fan_1_OnOff_Status_report, (bool)fan.get_rpm());
    }
  }
  //    cnt++;?
  //    can_object.read_messages();
  //    sleep_ms(100);
  //    fan_routine();
  // SERIAL_TO_USE.print(millis());
  //  auto time_start = micros();
  //  auto time_end = micros();
  //  SERIAL_TO_USE.printf("read_input took %d microseconds\r\n",
  //  time_end-time_start); SERIAL_TO_USE.println(temp_duty_cycle); if (i > -1)
  //  {
  //    user_input = i;
  //    SERIAL_TO_USE.println(i);
  //  }
}
