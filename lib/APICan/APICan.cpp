#include "APICan.h"
#include "Arduino.h"
#include "SPI.h"
#include "ACAN2515.h"

ACAN2515* APICan::pCan = nullptr;

void APICan::can_isr_routine() {
  pCan->isr();
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
// works! wow O_o
// constructor accepts CS and INT pin, forwards it to CAN constructor.
APICan::APICan(byte CS_PIN, byte INT_PIN, uint32_t quartz_frequency, Print* print):
    can(CS_PIN, SPI, INT_PIN), quartz_frequency(quartz_frequency), debugOutput{print}, int_pin{INT_PIN} {
  pCan = &can;
}

// interrupt pin wird nicht direkt in der method benötigt, alles schon initialisiert
// wieso eigentlich hier quartz frequenz extra reingeben und nicht im konstruktor :D
// ein relikt. ich habe alles mögliches ausprobiert.
void APICan::init(){ //der fehler sollte weg sein
  //hier kann man eigentlich can.begin() und so machen
}

void APICan::configure_chip() { //hm meeh extra den Serial port als argument.. besser referenz im constructor

  //--- Configure ACAN2515
  debugOutput->println("Configure ACAN2515");
  ACAN2515Settings settings(quartz_frequency,
                            125UL * 1000UL); // CAN bitrate 125 kb/s
  settings.mRequestedMode = ACAN2515Settings::NormalMode;
  uint16_t errorCode = 0;
  if(int_pin == 255) {
    errorCode = can.begin(settings, nullptr);
  } else {
      //falls interrupt pin benutzt wird
      errorCode = can.begin(settings, &APICan::can_isr_routine);
  }
  if (errorCode == 0) {
    debugOutput->print("Bit Rate prescaler: ");
    debugOutput->println(settings.mBitRatePrescaler);
    debugOutput->print("Propagation Segment: ");
    debugOutput->println(settings.mPropagationSegment);
    debugOutput->print("Phase segment 1: ");
    debugOutput->println(settings.mPhaseSegment1);
    debugOutput->print("Phase segment 2: ");
    debugOutput->println(settings.mPhaseSegment2);
    debugOutput->print("SJW: ");
    debugOutput->println(settings.mSJW);
    debugOutput->print("Triple Sampling: ");
    debugOutput->println(settings.mTripleSampling ? "yes" : "no");
    debugOutput->print("Actual bit rate: ");
    debugOutput->print(settings.actualBitRate());
    debugOutput->println(" bit/s");
    debugOutput->print("Exact bit rate ? ");
    debugOutput->println(settings.exactBitRate() ? "yes" : "no");
    debugOutput->print("Sample point: ");
    debugOutput->print(settings.samplePointFromBitStart());
    debugOutput->println("%");
  } else {
    debugOutput->print("Configuration error 0x");
    debugOutput->println(errorCode, HEX);
  }
}

// APICan::~APICan() = default;

void APICan::send_message(int id, uint8_t data[8]) {
  can.poll();
  CANMessage frame{};
  frame.id = id;   // id 0x000000
  frame.ext = true; // not an extended frame
  frame.len = 8;     // 8 user bytes
  // Move all 8 bytes from input to the frame
  for(auto i=0; i < 8; ++i){
    frame.data[i] = data[i];
  }
//  if (gBlinkLedDate < millis()) {
//    gBlinkLedDate += 200;
    digitalWrite(LED_BUILTIN, 1);
    const bool ok = can.tryToSend(frame);
    if (ok) {
      gSentFrameCount += 1;
      debugOutput->print("Sent from 2040 frame : #");
      debugOutput->println(gSentFrameCount);
      digitalWrite(LED_BUILTIN, 0);
    } else {
      debugOutput->println("Send failure");
    }
//  }
}

void APICan::read_messages() {
  if (can.available()) {
    CANMessage frame{};
    digitalWrite(LED_BUILTIN, 1);
    can.receive(frame);
    gReceivedFrameCount++;
    debugOutput->print("Received from Arduino value of: ");
    int buffer_int = frame.data_s64;
    String s_data_s64 = String(buffer_int, DEC);
    debugOutput->print(s_data_s64);
    debugOutput->print(" with: #");
    debugOutput->println(gReceivedFrameCount);
    sleep_ms(10);
    digitalWrite(LED_BUILTIN, 0);
    //    } else {
    //        debugOutput->println("Can BUS is not available for reading");
  }
}
