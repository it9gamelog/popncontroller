/*
 * Pop'n Arcade Controller
 * https://github.com/it9gamelog/popncontroller
 */

#pragma once

#include <stdint.h>
#include <Arduino.h>
#include <PluggableUSB.h>
#include <HID.h>

class PopnController_ : public PluggableUSBModule
{
  public:
    PopnController_(void);
    void press(char button);
    void release(char button);
    void send(void);
    uint16_t getLeds(void);
    
  protected:
    uint8_t epType[1];
    uint8_t protocol;
    uint8_t idle;

    struct __attribute__ ((packed)) {
      uint8_t report_id;
      uint16_t leds;
    } incoming;
    
    struct __attribute__ ((packed)) {
      uint8_t report_id;
      uint16_t keys;
    } report, last_report;

    int getInterface(uint8_t* interfaceCount);
    int getDescriptor(USBSetup& setup);
    bool setup(USBSetup& setup);
};

extern PopnController_ PopnController;
