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
// PIN DEFINITIONS
// SPI
#define INT_PIN 255 // INT output of MCP2515 (adapt to your design)
#define SCK_PIN 2   // SCK input of MCP2515
#define MOSI_PIN 3  // SDI input of MCP2515
#define MISO_PIN 4  // SDO output of MCP2517
#define CS_PIN 5    // CS input of MCP2515 (adapt to your design)

// Global objects initialization
APICan can_object(CS_PIN, INT_PIN, 16000000UL, &SERIAL_TO_USE);

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
  can_object.init();
  can_object.configure_chip();
}
uint8_t cnt = 0;
void loop() {
  int64_t message = millis();
  uint8_t payload[] = {cnt,0,1,1,0,0,0,0};
  can_object.send_message(0b0000000100, payload);
  cnt++;
//  can_object.read_messages();
  sleep_ms(100);
}
