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

  uint32_t quartz_frequency;
  Print* debugOutput;
  byte int_pin;

public:
  //constructor accepts CS and INT pins, default value of 20MHz for quartz, but explicit value can
  //be given, too.
  APICan(byte CS_PIN, byte INT_PIN, uint32_t quartz_frequency = 20000000UL, Print* debugOutput = &Serial);
  ACAN2515 can;

  static void can_isr_routine();
  static ACAN2515* pCan;

  void send_message(int id, int64_t data);

  void init();

 static void setSPI(unsigned char sck, unsigned char mosi,
                     unsigned char miso, unsigned char cs);

  void configure_chip();
  void read_messages();

};

#endif // FANCONTROLLER_APICAN_H
