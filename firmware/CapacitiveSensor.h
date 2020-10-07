/*
 * Pop'n Arcade Controller
 * https://github.com/it9gamelog/popncontroller
 * 
 * Inspried by CapacitiveSense.h v.04 - Capacitive Sensing Library for 'duino / Wiring    
 *   https://github.com/PaulStoffregen/CapacitiveSensor    
 *   http://www.pjrc.com/teensy/td_libs_CapacitiveSensor.html    
 *   http://playground.arduino.cc/Main/CapacitiveSensor    
 *   
 *   Copyright (c) 2009 Paul Bagder  All right reserved.    
 *   Version 05 by Paul Stoffregen - Support non-AVR board: Teensy 3.x, Arduino Due    
 *   Version 04 by Paul Stoffregen - Arduino 1.0 compatibility, issue 146 fix
 */

#pragma once

#include <stdint.h>
#include <Arduino.h>

// Direct I/O through registers and bitmask (from OneWire library)

#if defined(__AVR__)
#define PIN_TO_BASEREG(pin)             (portInputRegister(digitalPinToPort(pin)))
#define PIN_TO_BITMASK(pin)             (digitalPinToBitMask(pin))
#define IO_REG_TYPE uint8_t
#define DIRECT_READ(base, mask)         (((*(base)) & (mask)) ? 1 : 0)
#define DIRECT_MODE_INPUT(base, mask)   ((*((base)+1)) &= ~(mask), (*((base)+2)) &= ~(mask))
#define DIRECT_MODE_OUTPUT(base, mask)  ((*((base)+1)) |= (mask))
#define DIRECT_WRITE_LOW(base, mask)    ((*((base)+2)) &= ~(mask))
#define DIRECT_WRITE_HIGH(base, mask)   ((*((base)+2)) |= (mask))
#endif

class CapacitiveSensor
{
  public:
    CapacitiveSensor(uint8_t sendPin, uint8_t receivePin);
    int sense(uint8_t samples);

    int total;
    int baseline;

  private:
    // Send pin's ports and bitmask
    IO_REG_TYPE sBit;
    volatile IO_REG_TYPE *sReg;
    
    // Receive pin's ports and bitmask
    IO_REG_TYPE rBit;
    volatile IO_REG_TYPE *rReg;

    unsigned long lastCalibration;
    bool senseOnce(void);
};
