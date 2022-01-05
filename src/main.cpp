#include "APICan.h"
#include "Wire.h"

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

#define INT_PIN 255 // INT output of MCP2515 (adapt to your design)
#define SCK_PIN 2   // SCK input of MCP2515
#define MOSI_PIN 3  // SDI input of MCP2515
#define MISO_PIN 4  // SDO output of MCP2517
#define CS_PIN 5    // CS input of MCP2515 (adapt to your design)
APICan can_object(INT_PIN, 20000000UL);

void setup() {
  APICan::setSPI(SCK_PIN, MOSI_PIN, MISO_PIN, CS_PIN);
  // INIT SERIAL AND LED_BUILTIN
  SERIAL_TO_USE.setTX(12);
  SERIAL_TO_USE.setRX(13);
  //--- Switch on builtin led
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  //--- Start serial
  SERIAL_TO_USE.begin(115200);
  //--- Wait for serial (blink led at 10 Hz during waiting)
  while (!SERIAL_TO_USE) {
    delay(50);
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  }

  can_object.configure_chip(SERIAL_TO_USE);
}

void loop() {
  int64_t message = millis();
  can_object.send_message(1, message, SERIAL_TO_USE);
  sleep_ms(200);
}
