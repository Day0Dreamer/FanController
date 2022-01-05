#include "APICan.h"
#include "Arduino.h"
#include "SPI.h"
#include <ACAN2515.h>

APICan::APICan(byte interrupt, uint32_t quartz_frequency) {
  INT_PIN = interrupt;
  QUARTZ_FREQUENCY = quartz_frequency;

  //——————————————————————————————————————————————————————————————————————————————
  //  Setup connection to the chip
  //——————————————————————————————————————————————————————————————————————————————
}

void APICan::setSPI(const byte sck, const byte mosi, const byte miso,
                    const byte cs) {
  //--- There are no default SPI pins so they must be explicitly assigned
  SPI.setSCK(sck);
  SPI.setTX(mosi);
  SPI.setRX(miso);
  SPI.setCS(cs);
  //--- Begin SPI
  SPI.begin();
}

void APICan::configure_chip(SerialUART SerialPort) {
  //--- Configure ACAN2515
  SerialPort.println("Configure ACAN2515");
  ACAN2515Settings settings(QUARTZ_FREQUENCY,
                            125UL * 1000UL); // CAN bitrate 125 kb/s
  settings.mRequestedMode = ACAN2515Settings::NormalMode;
  const uint16_t errorCode = can.begin(settings, nullptr);
  //    const uint16_t errorCode = can.begin (settings, [] { can.isr () ; }) ;
  if (errorCode == 0) {
    SerialPort.print("Bit Rate prescaler: ");
    SerialPort.println(settings.mBitRatePrescaler);
    SerialPort.print("Propagation Segment: ");
    SerialPort.println(settings.mPropagationSegment);
    SerialPort.print("Phase segment 1: ");
    SerialPort.println(settings.mPhaseSegment1);
    SerialPort.print("Phase segment 2: ");
    SerialPort.println(settings.mPhaseSegment2);
    SerialPort.print("SJW: ");
    SerialPort.println(settings.mSJW);
    SerialPort.print("Triple Sampling: ");
    SerialPort.println(settings.mTripleSampling ? "yes" : "no");
    SerialPort.print("Actual bit rate: ");
    SerialPort.print(settings.actualBitRate());
    SerialPort.println(" bit/s");
    SerialPort.print("Exact bit rate ? ");
    SerialPort.println(settings.exactBitRate() ? "yes" : "no");
    SerialPort.print("Sample point: ");
    SerialPort.print(settings.samplePointFromBitStart());
    SerialPort.println("%");
  } else {
    SerialPort.print("Configuration error 0x");
    SerialPort.println(errorCode, HEX);
  }
}

// APICan::~APICan() = default;

void APICan::send_message(int id, int data, SerialUART SerialPort) {
  can.poll();
  CANMessage frame{};
  frame.id = 0x12;   // id 0x12
  frame.ext = false; // not an extended frame
  frame.len = 8;     // 2 user bytes
  frame.data_s64 = millis();
  if (gBlinkLedDate < millis()) {
    gBlinkLedDate += 200;
    digitalWrite(LED_BUILTIN, 1);
    const bool ok = can.tryToSend(frame);
    if (ok) {
      gSentFrameCount += 1;
      SerialPort.print("Sent from 2040 frame : #");
      SerialPort.println(gSentFrameCount);
      digitalWrite(LED_BUILTIN, 0);
    } else {
      SerialPort.println("Send failure");
    }
  }
}

void APICan::read_messages(SerialUART SerialPort) {
  if (can.available()) {
    CANMessage frame{};
    digitalWrite(LED_BUILTIN, 1);
    can.receive(frame);
    gReceivedFrameCount++;
    SerialPort.print("Received from Arduino value of: ");
    int buffer_int = frame.data_s64;
    String s_data_s64 = String(buffer_int, DEC);
    SerialPort.print(s_data_s64);
    SerialPort.print(" with: #");
    SerialPort.println(gReceivedFrameCount);
    sleep_ms(10);
    digitalWrite(LED_BUILTIN, 0);
    //    } else {
    //        SerialPort.println("Can BUS is not available for reading");
  }
}