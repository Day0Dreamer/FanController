#include "Wire.h"

#define NO_USB

#ifdef NO_USB
/* hardware UART*/
#define SERIAL_TO_USE Serial1
#else
/* USB */
#define SERIAL_TO_USE Serial
#endif

//——————————————————————————————————————————————————————————————————————————————
//  ACAN2515 Demo in loopback mode, for the Raspberry Pi Pico
//  Thanks to Duncan Greenwood for providing this sample sketch
//——————————————————————————————————————————————————————————————————————————————

#ifndef ARDUINO_ARCH_RP2040
#error "Select a Raspberry Pi Pico board"
#endif

//——————————————————————————————————————————————————————————————————————————————
#include "Arduino.h"
#include <ACAN2515.h>
#include "SPI.h"
//#include "SerialUART.h"

//——————————————————————————————————————————————————————————————————————————————
// The Pico has two SPI peripherals, SPI and SPI1. Either (or both) can be used.
// The are no default pin assignments to these must be set explicitly.
// At the time of writing (Apr 2021) there is no official Arduino core for the Pico
// Testing was done with Earle Philhower's arduino-pico core:
// https://github.com/earlephilhower/arduino-pico
// There is a small bug in release 1.0.3 so you will require at least 1.0.4
//——————————————————————————————————————————————————————————————————————————————

static const byte MCP2515_INT = 20    ;  // INT output of MCP2515 (adapt to your design)
static const byte MCP2515_SCK  = 2 ; // SCK input of MCP2515
static const byte MCP2515_MOSI = 3 ; // SDI input of MCP2515
static const byte MCP2515_MISO = 4 ; // SDO output of MCP2517
static const byte MCP2515_CS  = 5 ;  // CS input of MCP2515 (adapt to your design)

//——————————————————————————————————————————————————————————————————————————————
//  MCP2515 Driver object
//——————————————————————————————————————————————————————————————————————————————

ACAN2515 can (MCP2515_CS, SPI, MCP2515_INT) ;

//——————————————————————————————————————————————————————————————————————————————
//  MCP2515 Quartz: adapt to your design
//——————————————————————————————————————————————————————————————————————————————

static const uint32_t QUARTZ_FREQUENCY = 8UL * 1000UL * 1000UL ; // 8 MHz

//——————————————————————————————————————————————————————————————————————————————
//   SETUP
//——————————————————————————————————————————————————————————————————————————————

void setup();
void loop();

void setup () {

    SERIAL_TO_USE.setTX(12);
    SERIAL_TO_USE.setRX(13);

    //--- There are no default SPI pins so they must be explicitly assigned
    SPI.setSCK(MCP2515_SCK);
    SPI.setTX(MCP2515_MOSI);
    SPI.setRX(MCP2515_MISO);
    SPI.setCS(MCP2515_CS);
    //--- Begin SPI
    SPI.begin () ;


    //--- Switch on builtin led
    pinMode (LED_BUILTIN, OUTPUT) ;
    digitalWrite (LED_BUILTIN, HIGH) ;
    //--- Start serial
    SERIAL_TO_USE.begin (115200) ;
    //--- Wait for serial (blink led at 10 Hz during waiting)
    while (!SERIAL_TO_USE) {
        delay (50) ;
        digitalWrite (LED_BUILTIN, !digitalRead (LED_BUILTIN)) ;
    }
    //--- Configure ACAN2515
    SERIAL_TO_USE.println ("Configure ACAN2515") ;
    ACAN2515Settings settings (QUARTZ_FREQUENCY, 125UL * 1000UL) ; // CAN bit rate 125 kb/s
    settings.mRequestedMode = ACAN2515Settings::LoopBackMode ; // Select loopback mode
    const uint16_t errorCode = can.begin (settings, [] { can.isr () ; }) ;
    sleep_ms(500);
    if (errorCode == 0) {
        SERIAL_TO_USE.print ("Bit Rate prescaler: ") ;
        SERIAL_TO_USE.println (settings.mBitRatePrescaler) ;
        SERIAL_TO_USE.print ("Propagation Segment: ") ;
        SERIAL_TO_USE.println (settings.mPropagationSegment) ;
        SERIAL_TO_USE.print ("Phase segment 1: ") ;
        SERIAL_TO_USE.println (settings.mPhaseSegment1) ;
        SERIAL_TO_USE.print ("Phase segment 2: ") ;
        SERIAL_TO_USE.println (settings.mPhaseSegment2) ;
        SERIAL_TO_USE.print ("SJW: ") ;
        SERIAL_TO_USE.println (settings.mSJW) ;
        SERIAL_TO_USE.print ("Triple Sampling: ") ;
        SERIAL_TO_USE.println (settings.mTripleSampling ? "yes" : "no") ;
        SERIAL_TO_USE.print ("Actual bit rate: ") ;
        SERIAL_TO_USE.print (settings.actualBitRate ()) ;
        SERIAL_TO_USE.println (" bit/s") ;
        SERIAL_TO_USE.print ("Exact bit rate ? ") ;
        SERIAL_TO_USE.println (settings.exactBitRate () ? "yes" : "no") ;
        SERIAL_TO_USE.print ("Sample point: ") ;
        SERIAL_TO_USE.print (settings.samplePointFromBitStart ()) ;
        SERIAL_TO_USE.println ("%") ;
    } else {
        SERIAL_TO_USE.print ("Configuration error 0x") ;
        SERIAL_TO_USE.println (errorCode, HEX) ;
    }
}

//----------------------------------------------------------------------------------------------------------------------

static uint32_t gBlinkLedDate = 0 ;
static uint32_t gReceivedFrameCount = 0 ;
static uint32_t gSentFrameCount = 0 ;

//——————————————————————————————————————————————————————————————————————————————

void loop () {
    sleep_ms(500);
    CANMessage frame ;
    if (gBlinkLedDate < millis ()) {
        gBlinkLedDate += 200 ;
        digitalWrite (LED_BUILTIN, !digitalRead (LED_BUILTIN)) ;
        const bool ok = can.tryToSend (frame) ;
        if (ok) {
            gSentFrameCount += 1 ;
            SERIAL_TO_USE.print ("Sent from 2040 frame : #") ;
            SERIAL_TO_USE.println (gSentFrameCount) ;
        } else {
            SERIAL_TO_USE.println ("Send failure") ;
        }
    }
    sleep_ms(500);
//    CANMessage frame ;
    if (can.available ()) {
        can.receive (frame) ;
        gReceivedFrameCount ++ ;
        SERIAL_TO_USE.print ("Received from Arduino value of: ") ;
        int buffer_int = frame.data_s64;
        String s_data_s64 = String(buffer_int, DEC);
        SERIAL_TO_USE.print (s_data_s64) ;
        SERIAL_TO_USE.print (" with: #") ;
        SERIAL_TO_USE.println (gReceivedFrameCount) ;
        sleep_ms(10);
    } else {
        SERIAL_TO_USE.println("Can BUS is not available for reading");
    }
}

//——————————————————————————————————————————————————————————————————————————————

