/*
 * Pop'n Arcade Controller
 * https://github.com/it9gamelog/popncontroller
 */

#include "PopnController.h"

static const uint8_t _hidReportDescriptor[] PROGMEM = {
  // Buttons Input
  0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
  0x09, 0x06,        // Usage (Keyboard)
  0xA1, 0x01,        // Collection (Application)
  0x85, 0x01,        //   Report ID (1)
  0x05, 0x07,        //   Usage Page (Kbrd/Keypad)

  // Total: 2 bytes
  // Buttons 0-8 (1 bit each)
  //   This defines the keys mapping for each button.
  //   Seems the most commonly used config
  0x09, 0x06,        //   Usage (0x06) C
  0x09, 0x09,        //   Usage (0x09) F
  0x09, 0x19,        //   Usage (0x19) V
  0x09, 0x0A,        //   Usage (0x0A) G
  0x09, 0x05,        //   Usage (0x05) B
  0x09, 0x0B,        //   Usage (0x0B) H
  0x09, 0x11,        //   Usage (0x11) N
  0x09, 0x0D,        //   Usage (0x0D) J
  0x09, 0x10,        //   Usage (0x10) M
  // Aux 0-2 (1 bit each)
  //   Not much thought on key mapping
  0x09, 0x14,        //   Usage (0x14) Q
  0x09, 0x1A,        //   Usage (0x1A) W
  0x09, 0x08,        //   Usage (0x08) E
  0x15, 0x00,        //   Logical Minimum (0)
  0x25, 0x01,        //   Logical Maximum (1)
  0x75, 0x01,        //   Report Size (1)
  0x95, 0x0C,        //   Report Count (12)
  0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
  // Padding (4 bits)
  0x75, 0x04,        //   Report Size (4)
  0x95, 0x01,        //   Report Count (1)
  0x81, 0x03,        //   Input (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
  0xC0,              // End Collection

  // LEDs Output
  0x05, 0x0C,        // Usage Page (Consumer)
  0x09, 0x01,        // Usage (Consumer Control)
  0xA1, 0x01,        // Collection (Application)
  0x85, 0x02,        //   Report ID (2)
  0x06, 0xFF, 0x07,  //   Usage Page (Reserved 0x07FF)

  // Total: 2 bytes
  // LEDs 0-8 (1 bit each)
  0x19, 0x01,        //   Usage Minimum (0x01)
  0x29, 0x09,        //   Usage Maximum (0x09)
  0x15, 0x00,        //   Logical Minimum (0)
  0x25, 0x01,        //   Logical Maximum (1)
  0x75, 0x01,        //   Report Size (1)
  0x95, 0x09,        //   Report Count (9)
  0x91, 0x02,        //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
  // Padding (7 bits)
  0x75, 0x07,        //   Report Size (7)
  0x95, 0x01,        //   Report Count (1)
  0x91, 0x03,        //   Output (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)

  // Some kind of input is needed for HID to be recognized, but not really used
  0x09, 0x00,        //   Usage (0x00)
  0x75, 0x08,        //   Report Size (8)
  0x95, 0x01,        //   Report Count (1)
  0x81, 0x03,        //   Input (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
  0xC0,              // End Collection
};

PopnController_::PopnController_(void) : PluggableUSBModule(1, 1, epType)
{
  last_report.keys = 0xffff;
  report.report_id = 1;
  report.keys = 0;
  incoming.leds = 0;

  epType[0] = EP_TYPE_INTERRUPT_IN;
  PluggableUSB().plug(this);
}

int PopnController_::getInterface(uint8_t* interfaceCount)
{
  *interfaceCount += 1; // uses 1
  HIDDescriptor hidInterface = {
    D_INTERFACE(pluggedInterface, 1, USB_DEVICE_CLASS_HUMAN_INTERFACE, HID_SUBCLASS_NONE, HID_PROTOCOL_NONE),
    D_HIDREPORT(sizeof(_hidReportDescriptor)),
    D_ENDPOINT(USB_ENDPOINT_IN(pluggedEndpoint), USB_ENDPOINT_TYPE_INTERRUPT, USB_EP_SIZE, 0x01)
  };
  return USB_SendControl(0, &hidInterface, sizeof(hidInterface));
}

int PopnController_::getDescriptor(USBSetup& setup)
{
  // Check if this is a HID Class Descriptor request
  if (setup.bmRequestType != REQUEST_DEVICETOHOST_STANDARD_INTERFACE) {
    return 0;
  }
  if (setup.wValueH != HID_REPORT_DESCRIPTOR_TYPE) {
    return 0;
  }

  // In a HID Class Descriptor wIndex cointains the interface number
  if (setup.wIndex != pluggedInterface) {
    return 0;
  }

  // Reset the protocol on reenumeration. Normally the host should not assume the state of the protocol
  // due to the USB specs, but Windows and Linux just assumes its in report mode.
  protocol = HID_REPORT_PROTOCOL;

  return USB_SendControl(TRANSFER_PGM, _hidReportDescriptor, sizeof(_hidReportDescriptor));
}

bool PopnController_::setup(USBSetup& setup)
{
  if (pluggedInterface != setup.wIndex) {
    return false;
  }

  uint8_t request = setup.bRequest;
  uint8_t requestType = setup.bmRequestType;

  if (requestType == REQUEST_DEVICETOHOST_CLASS_INTERFACE)
  {
    if (request == HID_GET_REPORT) {
      // TODO: HID_GetReport();
      return true;
    }
    if (request == HID_GET_PROTOCOL) {
      // TODO: Send8(protocol);
      return true;
    }
  }

  if (requestType == REQUEST_HOSTTODEVICE_CLASS_INTERFACE)
  {
    if (request == HID_SET_PROTOCOL) {
      protocol = setup.wValueL;
      return true;
    }
    if (request == HID_SET_IDLE) {
      idle = setup.wValueL;
      return true;
    }
    if (request == HID_SET_REPORT)
    {
      // Check if data has the correct length
      auto length = setup.wLength;
      if (length == sizeof(incoming)) {
        USB_RecvControl(&incoming, length);
      }
      return true;
    }
  }

  return false;
}

void PopnController_::press(char button) {
  report.keys |= 1 << button;
}

void PopnController_::release(char button) {
  report.keys &= ~(1 << button);
}

uint16_t PopnController_::getLeds(void) {
  return incoming.leds;
}

void PopnController_::send(void) {
  if (last_report.keys != report.keys) {
    USB_Send(pluggedEndpoint | TRANSFER_RELEASE, &report, sizeof(report));
    last_report.keys = report.keys;
  }
}

PopnController_ PopnController;
