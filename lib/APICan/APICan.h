#ifndef FANCONTROLLER_APICAN_H
#define FANCONTROLLER_APICAN_H

#include <variant>
#include <any>
#include <map>
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
  APICan(byte CS_PIN, byte INT_PIN, uint32_t quartz_frequency, Print* debugOutput = &Serial);
  ACAN2515 can;

  static void can_isr_routine();
  static ACAN2515* pCan;

  void send_message(int id, int64_t data);

  void init();

 static void setSPI(unsigned char sck, unsigned char mosi,
                     unsigned char miso, unsigned char cs);

 int configure_chip();
  void read_messages();

};

class BusAddress {

  // Priority Placeholder2 Placeholder1 Device   Index# MessageType MessageID
  // 000      000000       000          000000   0000   00          00000             BINARY
  // Max 8    Max 64       Max 8        Max 64   Max 16 Max 4       Max 32            Max values
  // 0:       0:           0:           53:      12:    1:          2                 DECIMAL
  // 0:       :            :            Fan:     8:     Command:    ChangeDutyCycle   STRING

private:
  std::map<String, std::any> split_address_in_parts(uint32_t address);
public:
  // Collection of device names and their CAN bus IDs
  static std::map<String, int> list_of_devices;
  static std::map<String, int> list_of_message_types;
  // Decodes binary address value to human-readable form
  String decode_as_string(uint address);
  std::map<String, std::any> decode(uint address);
  // split binary address in chunks
      // Work through each chunk
      // Return as a collection of numbers and class references
  // Encodes human-readable address form to binary value
  uint encode(String address);
  // Returns assigned numbers for each device type
  uint get_device_type_number(String device_name);
  String get_device_type_name(std::any device_number);
  String get_message_type_name(std::any message_number);
};

#endif // FANCONTROLLER_APICAN_H
