#ifndef FANCONTROLLER_APICAN_H
#define FANCONTROLLER_APICAN_H

#include "ACAN2515.h"
#include "Arduino.h"
#include "SPI.h"

class APICan {
private:
  uint32_t gBlinkLedDate = 0;
  uint32_t gReceivedFrameCount = 0;
  uint32_t gSentFrameCount = 0;

  byte CS_PIN = 0;
  byte INT_PIN = 0;
  uint QUARTZ_FREQUENCY = 20000000UL;

public:
  APICan(byte interrupt, uint32_t quartz_frequency);
  ACAN2515 can = ACAN2515(CS_PIN, SPI, INT_PIN);

  void send_message(int id, int data, SerialUART SerialPort);

  static void setSPI(unsigned char sck, unsigned char mosi,
                     unsigned char miso, unsigned char cs);

  void configure_chip(SerialUART SerialPort);
  void read_messages(SerialUART SerialPort);
};

#endif // FANCONTROLLER_APICAN_H
