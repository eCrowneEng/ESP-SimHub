

#pragma once

#include "USBHID.h"

#if SOC_USB_OTG_SUPPORTED
#if CONFIG_TINYUSB_HID_ENABLED
#define MAX_BUTTONS 128

struct JoystickDescription {
  uint16_t vendorId;
  uint16_t productId;
  const char *name;
  const char *manufacturer;
};

class ECrowneJoystick : public USBHIDDevice {
private:
  USBHID hid;
  uint16_t _t;          ///< Delta x  movement of left analog-stick
  uint16_t _a;          ///< Delta y  movement of left analog-stick
  uint16_t _b;          ///< Delta z  movement of right analog-joystick
  uint8_t _buttons[16]; 
  bool write();

public:
  ECrowneJoystick(void);
  void begin(JoystickDescription description);
  void end(void);

  void setThrottle(uint16_t t);
  void setAccelerator(uint16_t a);
  void setBrake(uint16_t b);

  // Set button as pressed and send state
  bool pressButton(uint8_t button);
  // Set button as released and send state
  bool releaseButton(uint8_t button);
  // Set button state without sending state
  bool setButton(uint8_t button, bool state);
  // Send state
  bool sendState();

  // internal use
  uint16_t _onGetDescriptor(uint8_t *buffer);
};

// https://www.usb.org/sites/default/files/hut1_5.pdf
#define HID_USAGE_SIMULATION_THROTTLE 0xBB
#define HID_USAGE_SIMULATION_ACCELERATOR 0xC4
#define HID_USAGE_SIMULATION_BRAKE 0xC5

/**
 * This is how we're defining the report descriptor for the joystick.
 * 
 * We're using the same pattern as the espressif gamepad library and reusing some of the constants there
 * 
 * The report descriptor is a byte array that describes the data that the device is sending to the host.
 * The report itself is also a byte array that conforms to this descriptor contract
 */
#define TUD_HID_REPORT_DESC_ECROWNE_JOYSTICK(...) \
  HID_USAGE_PAGE ( HID_USAGE_PAGE_DESKTOP     )                 ,\
  HID_USAGE      ( HID_USAGE_DESKTOP_JOYSTICK )                 ,\
  HID_COLLECTION ( HID_COLLECTION_APPLICATION )                 ,\
    /* Report ID if any */\
    __VA_ARGS__ \
    \
    /* 16x8=128 bit Button Map */ \
    HID_USAGE_PAGE     ( HID_USAGE_PAGE_BUTTON                  ) ,\
    HID_USAGE_MIN      ( 1                                      ) ,\
    HID_USAGE_MAX      ( MAX_BUTTONS                            ) ,\
    HID_LOGICAL_MIN    ( 0                                      ) ,\
    HID_LOGICAL_MAX    ( 1                                      ) ,\
    HID_REPORT_SIZE    ( 1                                      ) ,\
    HID_REPORT_COUNT   ( MAX_BUTTONS                            ) ,\
    HID_UNIT_EXPONENT  ( 0                                      ) ,\
    HID_UNIT           ( 0                                      ) ,\
    HID_INPUT          ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE ) ,\
    /* 16 bit axis (0-65535) */ \
    HID_USAGE_PAGE     ( HID_USAGE_PAGE_SIMULATE                ) ,\
    /* Minimum report from axis */ \
    HID_LOGICAL_MIN    ( 0                                      ) ,\
    /* Maximum report from axis */ \
    HID_LOGICAL_MAX_N  ( 65535, 3                               ) ,\
    /* Size of each report from axis */ \
    HID_REPORT_SIZE    ( 16                                     ) ,\
    /* number of axis */ \
    HID_REPORT_COUNT   ( 3                                      ) ,\
    HID_COLLECTION     ( HID_COLLECTION_PHYSICAL                ) ,\
    HID_USAGE          ( HID_USAGE_SIMULATION_THROTTLE          ) ,\
    HID_USAGE          ( HID_USAGE_SIMULATION_ACCELERATOR       ) ,\
    HID_USAGE          ( HID_USAGE_SIMULATION_BRAKE             ) ,\
    HID_INPUT          ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE ) ,\
    HID_COLLECTION_END ,\
  HID_COLLECTION_END
    
    



typedef struct TU_ATTR_PACKED
{
  uint8_t buttons[16];  ///< Buttons mask for currently pressed buttons, supports 128 (16x8)
  uint16_t t;            ///< Delta throttle
  uint16_t a;            ///< Delta accelerator
  uint16_t b;            ///< Delta brake
} hid_joystick_report_t;


#endif /* CONFIG_TINYUSB_HID_ENABLED */
#endif /* SOC_USB_OTG_SUPPORTED */
