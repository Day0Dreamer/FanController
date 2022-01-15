#include <APICan.h>
#include <APIFan.h>
#include <stdio.h>
#include "pico/stdlib.h"

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

void setup();
void loop();
// PIN DEFINITIONS
// SPI
#define INT_PIN 255 // INT output of MCP2515 (adapt to your design)
#define SCK_PIN 2   // SCK input of MCP2515
#define MOSI_PIN 3  // SDI input of MCP2515
#define MISO_PIN 4  // SDO output of MCP2517
#define CS_PIN 5    // CS input of MCP2515 (adapt to your design)

// Global objects initialization
APICan can_object(CS_PIN, INT_PIN, 16000000UL, &SERIAL_TO_USE);

extern "C" {
#include "pico/stdlib.h"
#include "pico/stdio_uart.h"
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


void setup() {
  // Init SPI Bus
  APICan::setSPI(SCK_PIN, MOSI_PIN, MISO_PIN, CS_PIN);
  // Init Serial
  SERIAL_TO_USE.setTX(0);
  SERIAL_TO_USE.setRX(1);
  // Start serial
  SERIAL_TO_USE.begin(115200);
  // Init blinking LED
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
#ifdef WAIT_FOR_SERIAL
  // Wait for serial (blink led at 10 Hz during waiting)
  while (!SERIAL_TO_USE) {
    delay(50);
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  }
#endif
  // Init CAN2515 module
  //  can_object.init();
  //  can_object.configure_chip();
  SERIAL_TO_USE.println("Booted up");

  // Init APIFan
  fan_setup();
  struct repeating_timer timer;
//  add_repeating_timer_ms(5000, repeating_timer_callback, nullptr, &timer);
//  SERIAL_TO_USE.println("Added repeating timer");

}

uint8_t user_input_buffer;
float temp_duty_cycle_main = .75;
int read_input(float &temp_duty_cycle) {
  if (SERIAL_TO_USE.available()) {
    user_input_buffer = SERIAL_TO_USE.read();
    if (user_input_buffer == '`') {
      temp_duty_cycle = 0.0;
      set_all_fans_to(temp_duty_cycle);
      SERIAL_TO_USE.printf("All fans has been set to %.2f\r\n",temp_duty_cycle);
      return 0;
    }
    if (user_input_buffer == '0') {
      temp_duty_cycle = 1.0;
      set_all_fans_to(temp_duty_cycle);
      SERIAL_TO_USE.printf("All fans has been set to %.2f\r\n",temp_duty_cycle);
      return 0;
    }
    else if (user_input_buffer == '5') {
      temp_duty_cycle = 0.5;
      set_all_fans_to(temp_duty_cycle);
      SERIAL_TO_USE.printf("All fans has been set to %.2f\r\n",temp_duty_cycle);
      return 0;
    }
    else if (user_input_buffer == '1') {
      temp_duty_cycle = 0.1;
      set_all_fans_to(temp_duty_cycle);
      SERIAL_TO_USE.printf("All fans has been set to %.2f\r\n",temp_duty_cycle);
      return 0;
    }
    else if (user_input_buffer == '-') {
      temp_duty_cycle -= 0.01;
      set_all_fans_to(temp_duty_cycle);
      SERIAL_TO_USE.printf("All fans has been set to %.2f\r\n",temp_duty_cycle);
      return 0;
    }
    else if (user_input_buffer == '=') {
      temp_duty_cycle += 0.01;
      set_all_fans_to(temp_duty_cycle);
      SERIAL_TO_USE.printf("All fans has been set to %.2f\r\n",temp_duty_cycle);
      return 0;
    }
    else if (user_input_buffer == 'f') {
      SERIAL_TO_USE.printf("══════════════════════════════════════════════════════════════════════\r\n",temp_duty_cycle);
      read_all_fans();
      SERIAL_TO_USE.printf("══════════════════════════════════════════════════════════════════════\r\n",temp_duty_cycle);
      return 0;
    }
    if (user_input_buffer == '\r' || user_input_buffer == '\n') {
      SERIAL_TO_USE.print("\n\r");
    }
  }
  return 0;
}

void loop() {
  read_input(temp_duty_cycle_main);
  generate_pwm_cycle();
  update_rpm_all_fans();




  //  int64_t message = millis();
  //  uint8_t payload[] = {cnt,0,1,1,0,0,0,0};
  //  can_object.send_message(0b0000000100, payload);
  //  cnt++;
  //  can_object.read_messages();
  //  sleep_ms(100);
  //  fan_routine();
  // SERIAL_TO_USE.print(millis());
//  auto time_start = micros();
//  auto time_end = micros();
//  SERIAL_TO_USE.printf("read_input took %d microseconds\r\n", time_end-time_start);
//  SERIAL_TO_USE.println(temp_duty_cycle);
//  if (i > -1) {
//    user_input = i;
//    SERIAL_TO_USE.println(i);
//  }
}
