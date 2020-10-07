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

#include "CapacitiveSensor.h"
#define CS_Timeout (8192)

CapacitiveSensor::CapacitiveSensor(uint8_t sendPin, uint8_t receivePin)
{  
  pinMode(sendPin, OUTPUT);   // sendpin to OUTPUT
  pinMode(receivePin, INPUT); // receivePin to INPUT
  digitalWrite(sendPin, LOW);
  
  sBit = PIN_TO_BITMASK(sendPin); // get send pin's ports and bitmask
  sReg = PIN_TO_BASEREG(sendPin);

  rBit = PIN_TO_BITMASK(receivePin);
  rReg = PIN_TO_BASEREG(receivePin);

  // Charge all pin to Low
  DIRECT_WRITE_LOW(sReg, sBit);
  DIRECT_WRITE_LOW(rReg, rBit);
  DIRECT_MODE_OUTPUT(rReg, rBit);
  // receivePin to INPUT (pullup is off)
  DIRECT_MODE_INPUT(rReg, rBit); // receivePin to INPUT (pullup is off)

  // input large value for calibration to work
  baseline = 0x7FFF;
  lastCalibration = millis();
}

int CapacitiveSensor::sense(uint8_t samples)
{
  total = 0;
  if (samples == 0)
    return 0;

  // loop for samples parameter - simple lowpass filter
  for (uint8_t i = 0; i < samples; i++) {
    if (!senseOnce()) return -2; // variable over timeout
  }

  // Baseline calibration
  // total is linearly related to the time required to charge the capacitance
  if (total < baseline - (int)samples * 2)
  {
    // The capacitance is much lower than what have known.
    // Reset immediately
    baseline = total;
  } else if (total < baseline + (int)samples * 2)
  {
    // If the touch is considered not touched...
    if (millis() - lastCalibration > 25)
    {
      // Slowly walk the baseline uphill/downhill to compensate the capacitance drift
      baseline += (total > baseline) ? 1 : -1;
      lastCalibration = millis();
    }
  }
  return total - baseline;
}

bool CapacitiveSensor::senseOnce(void)
{  
  // Backup global interrupt state 
  auto oldSREG = SREG;
  
  // sendPin High
  DIRECT_WRITE_HIGH(sReg, sBit);
  
  // Disable interrupt for timing.
  // This function will finish in 10 microseconds or so.  
  noInterrupts();

  // Keep counting when Low is still being detected
  while (!DIRECT_READ(rReg, rBit) && (total < CS_Timeout)) { 
    total++;
  }

  // Now charge the pin to HIGH fully and count again
  // receivePin to High
  DIRECT_WRITE_HIGH(rReg, rBit);
  DIRECT_MODE_OUTPUT(rReg, rBit);
  delayMicroseconds(5);
  // receivePin to INPUT (pullup is off)
  DIRECT_MODE_INPUT(rReg, rBit); 
  // sendPin LOW
  DIRECT_WRITE_LOW(sReg, sBit);

  // Keep counting when High is still being detected
  while (DIRECT_READ(rReg, rBit) && (total < CS_Timeout)) { 
    total++;
  }
  
  // Restore interrupt
  SREG = oldSREG;
  return total < CS_Timeout;
}
