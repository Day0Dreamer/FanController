#include "APICan.h"
#include "ACAN2515.h"
#include "Arduino.h"
#include "SPI.h"

ACAN2515 *APICan::pCan = nullptr;

void APICan::can_isr_routine() { pCan->isr(); }

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

APICan::APICan(byte CS_PIN, byte INT_PIN, uint32_t quartz_frequency,
               Print *print)
    : can(CS_PIN, SPI, INT_PIN),
      quartz_frequency(quartz_frequency), debugOutput{print}, int_pin{INT_PIN} {
  pCan = &can;
}

void APICan::init() {
}

int APICan::configure_chip() {

  //--- Configure ACAN2515
  debugOutput->println("Configure ACAN2515");
  ACAN2515Settings settings(quartz_frequency,
                            125UL * 1000UL); // CAN bitrate 125 kb/s
  settings.mRequestedMode = ACAN2515Settings::NormalMode;
  uint16_t errorCode = 0;
  if (int_pin == 255) {
    errorCode = can.begin(settings, nullptr);
  } else {
    // falls interrupt pin benutzt wird
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
  return errorCode;
}

void APICan::send_message(int id, int64_t data) {
  can.poll();
  CANMessage frame{};
  frame.id = id;    // id 0x000000
  frame.ext = true; // extended frame?
  frame.len = 8;    // 8 user bytes
  frame.data_s64 = (int64_t)data;

  digitalWrite(LED_BUILTIN, 1);
  const bool ok = can.tryToSend(frame);
  for (auto i = 0; i < 10; ++i) {
    if (ok) {
      gSentFrameCount += 1;
      debugOutput->printf("Sent with ID:%d, payload %i, frame#:", frame.id,
                          frame.data_s64);
      debugOutput->println(gSentFrameCount);
      digitalWrite(LED_BUILTIN, 0);
      break;
    } else {
      debugOutput->printf("Try#%2d Send failure\r\n", i+1);
    }
  }
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

std::map<String, int> BusAddress::list_of_devices = {
    {"fan", 0},
    {"ac", 1},
    {"generator", 2}
};

std::map<String, int> BusAddress::list_of_message_types = {
    {"status_report", 0},
    {"command", 1},
    {"acknowledgement", 2},
    {"placeholder", 3}
};

uint BusAddress::get_device_type_number(String device_name) {
  for (const auto& [key, value] : this->list_of_devices) {
     if (key == device_name) return value;
  }
  return -1;
}
std::map<String, std::any> BusAddress::split_address_in_parts(uint32_t address) {
  uint32_t const mask_priority =          0b111'000000'000'000000'0000'00'00000;
  uint32_t const mask_placeholder2 =      0b000'111111'000'000000'0000'00'00000;
  uint32_t const mask_placeholder1 =      0b000'000000'111'000000'0000'00'00000;
  uint32_t const mask_device_type =       0b000'000000'000'111111'0000'00'00000;
  uint32_t const mask_device_index =      0b000'000000'000'000000'1111'00'00000;
  uint32_t const mask_message_type =      0b000'000000'000'000000'0000'11'00000;
  uint32_t const mask_message_id =        0b000'000000'000'000000'0000'00'11111;

  return {
    { "priority", address &mask_priority },
    { "placeholder1", address & mask_placeholder1},
    { "placeholder2", address & mask_placeholder2},
    { "device_type", address & mask_device_type},
    { "device_index", address & mask_device_index},
    { "message_type", address & mask_message_type},
    { "message_id", address & mask_message_id},
  };
}

std::map<String, std::any> BusAddress::decode(uint address) {
  std::map<String, std::any> address_parts = this->split_address_in_parts(address);
  //  address_parts["priority"],
  //  address_parts["placeholder1"],
  //  address_parts["placeholder2"],
  address_parts["device_type"] = this->get_device_type_name(address_parts["device_type"]);
  //  address_parts["device_index"],
  address_parts["message_type"] = this->get_message_type_name(address_parts["message_type"]);
  //  address_parts["message_id"]

  return address_parts;
}

// Can't figure out how to convert to string
//String BusAddress::decode_as_string(uint address) {
//  std::map<String, std::any> address_parts = this->decode(address);
//  String result_string = "";
//  for (auto &part : address_parts) {
//    result_string = result_string + (String)part + ":";
//    // todo: add skip for the last ":"
//  }
//  return result_string;
//}

String BusAddress::get_device_type_name(std::any device_number) {
  for (auto const& [key, val] : this->list_of_devices) {
    if (val == std::any_cast<int>(device_number)) {return key;}
  }
  return "";
}

String BusAddress::get_message_type_name(std::any message_number) {
  for (auto const& [key, val] : this->list_of_message_types) {
    if (val == std::any_cast<int>(message_number)) {return key;}
  }
  return "";
}

uint BusAddress::encode(String address) { return 0; }
