#include <APICan.h>
#include <APIFan.h>


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
  stdio_uart_init_full(uart0, 115200, 12, 13);
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


void setup() {
  // Init SPI Bus
  APICan::setSPI(SCK_PIN, MOSI_PIN, MISO_PIN, CS_PIN);
  // Init Serial
  SERIAL_TO_USE.setTX(12);
  SERIAL_TO_USE.setRX(13);
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
}
uint8_t cnt = 0;

uint8_t buffer;

double temp_duty_cycle = .75;
void read_input(double& temp_duty_cycle) {
  if (SERIAL_TO_USE.available()) {
    buffer = SERIAL_TO_USE.read();
    if (buffer == '`') {
      temp_duty_cycle = 0.0;
    }
    if (buffer == '0') {
      temp_duty_cycle = 1.0;
    }
    else if (buffer == '5') {
      temp_duty_cycle = 0.5;
    }
    else if (buffer == '1') {
      temp_duty_cycle = 0.1;
    }
    else if (buffer == '-') {
      temp_duty_cycle -= 0.01;
    }
    else if (buffer == '=') {
      temp_duty_cycle += 0.01;
    }
    if (buffer == '\r' || buffer == '\n') {
      SERIAL_TO_USE.print("\n\r");
    }
    SERIAL_TO_USE.println(temp_duty_cycle);

  }
}

float user_input;
uint64_t loop_counter = 0;
void loop() {
//  ++loop_counter;
  //  int64_t message = millis();
  //  uint8_t payload[] = {cnt,0,1,1,0,0,0,0};
  //  can_object.send_message(0b0000000100, payload);
  //  cnt++;
  //  can_object.read_messages();
  //  sleep_ms(100);
  //  fan_routine();
  // SERIAL_TO_USE.print(millis());
//  auto time_start = micros();
  read_input(temp_duty_cycle);
//  auto time_end = micros();
//  SERIAL_TO_USE.printf("read_input took %d microseconds\r\n", time_end-time_start);
//  SERIAL_TO_USE.println(temp_duty_cycle);
//  if (i > -1) {
//    user_input = i;
//    SERIAL_TO_USE.println(i);
//  }
  generate_pwm_cycle();
  //  repeat_input();
//  repeat_back_pico_sdk();
}
