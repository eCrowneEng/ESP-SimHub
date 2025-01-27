#include "USBHID.h"

#if SOC_USB_OTG_SUPPORTED
#if CONFIG_TINYUSB_HID_ENABLED

#include <ECrowneJoystick.h>
#include <Arduino.h>
#include "USB.h"

static const uint8_t report_descriptor[] = {TUD_HID_REPORT_DESC_ECROWNE_JOYSTICK(HID_REPORT_ID(HID_REPORT_ID_GAMEPAD))};


ECrowneJoystick::ECrowneJoystick(): hid(), _t(0), _a(0), _b(0) {
  static bool initialized = false;
  memset(_buttons, 0, sizeof(_buttons));
  if (!initialized) {
    initialized = true;

    // Add device with the correct size
    hid.addDevice(this, sizeof(report_descriptor));
  }
}

uint16_t ECrowneJoystick::_onGetDescriptor(uint8_t *dst) {
  memcpy(dst, report_descriptor, sizeof(report_descriptor));
  return sizeof(report_descriptor);
}

void ECrowneJoystick::begin(JoystickDescription description) {
  USB.VID(description.vendorId);
  USB.PID(description.productId);
  USB.productName(description.name);
  USB.manufacturerName(description.manufacturer);
  USB.begin();

  hid.begin();
}

void ECrowneJoystick::end() {}

bool ECrowneJoystick::write() {
  hid_joystick_report_t report;
  memset(&report, 0, sizeof(report));
  
  memcpy(report.buttons, _buttons, sizeof(report.buttons));
  
  // Set the axes values
  report.t = _t;
  report.a = _a;
  report.b = _b;

  return hid.SendReport(HID_REPORT_ID_GAMEPAD, &report, sizeof(report));
}

void ECrowneJoystick::setThrottle(uint16_t t) {
  _t = t;
}

void ECrowneJoystick::setAccelerator(uint16_t a) {
  _a = a;
}

void ECrowneJoystick::setBrake(uint16_t b) {
  _b = b;
}

bool ECrowneJoystick::setButton(uint8_t button, bool state) {
  if (button >= MAX_BUTTONS) {
    return false;
  }
  uint8_t byte_index = button / 8;
  uint8_t bit_position = button % 8;
  if (state) {
    _buttons[byte_index] |= (1 << bit_position);
  } else {
    _buttons[byte_index] &= ~(1 << bit_position);
  }
  return true;
}
  
bool ECrowneJoystick::sendState() {
  return write();
}

bool ECrowneJoystick::pressButton(uint8_t button) {
  setButton(button, true);
  return write();
}

bool ECrowneJoystick::releaseButton(uint8_t button) {
  setButton(button, false);
  return write();
}

#endif /* CONFIG_TINYUSB_HID_ENABLED */
#endif /* SOC_USB_OTG_SUPPORTED */
