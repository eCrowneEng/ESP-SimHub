/*
 * simhub_standard_protocol.h
 * -----------------------------------------------------------------------------
 * SIMHUB_1.4 "standard serial protocol" responder for ESP-SimHub.
 *
 * Why this file exists:
 *   SimHub >= 9.12 identifies and drives devices configured through a *device
 *   definition* with plain ASCII commands prefixed by 6 consecutive 0xFF bytes,
 *   and expects the protocol version string "SIMHUB_1.4". A firmware speaking
 *   only the classic Arduino (ARQ) protocol never answers those commands, so
 *   SimHub reports 'NoResponse' / 'InvalidFirmwareVersion' and the device never
 *   appears in the custom device page.
 *
 * Reference implementation (authoritative, shipped with SimHub):
 *   <SimHub>\_Addons\Arduino\ProMicroSimhubStandardDriver\protocol.h
 *   PROTOCOLVERSION "SIMHUB_1.4", matrix decoding in matrixcore.h
 *
 * Wire format (host -> device):
 *   (0xFF)x6 + 5 ASCII chars                      e.g. "proto", "snumb"
 *   (0xFF)x6 + "smatx" + 64*3 RGB + FF FE FD      matrix frame
 *   (0xFF)x6 + "sleds" + N*3 RGB + FF FE FD       led strip frame
 * All informational replies are plain ASCII + CRLF.
 *
 * Both protocols coexist on the same port: classic ARQ frames start with
 * 0x01 0x01, the standard protocol starts with a run of 0xFF. The classic
 * parser resynchronises on its own when it sees standard-protocol traffic.
 *
 * Enable with -D ENABLE_SIMHUB14=1 (see platformio.ini, env:esp32c3).
 * -----------------------------------------------------------------------------
 */

#pragma once

#include <Arduino.h>

// Enabled with -D ENABLE_SIMHUB14=1 (see platformio.ini, env:esp32c3).
// When it is off, the two stubs at the bottom of this file keep the call sites
// in the core files valid and inert, so they need no conditional compilation.
#ifdef ENABLE_SIMHUB14

#include <EspSimHub.h>  // getUniqueId() -> WiFi.macAddress(), same id the classic protocol reports

// ------------------------- device configuration ------------------------------

#ifndef SH14_DEVICE_NAME
#ifdef DEVICE_NAME
#define SH14_DEVICE_NAME DEVICE_NAME
#else
#define SH14_DEVICE_NAME "ESP-SimHub Device"
#endif
#endif

#ifndef SH14_DEVICE_BRAND
#define SH14_DEVICE_BRAND "ECrowne"
#endif

#ifndef SH14_DEVICE_PICTURE_URL
#define SH14_DEVICE_PICTURE_URL "https://simhubdash.com/devices/nopicture.png"
#endif

// Device types (enums.h of the standard driver):
//   0 unspecified, 1 leds, 11 matrix, 12 matrix+leds, 21 fans
#ifndef SH14_DEVICE_TYPE
#define SH14_DEVICE_TYPE 11
#endif

// 0: device is bound through a device definition (VID/PID + serial picker)
#ifndef SH14_DEVICE_AUTODETECT_ALLOWED
#define SH14_DEVICE_AUTODETECT_ALLOWED 0
#endif

#ifndef SH14_SCREEN_AND_LEDS_RELATION
#define SH14_SCREEN_AND_LEDS_RELATION 0
#endif

// Upload protection is an AVR watchdog trick, meaningless on ESP32. Report the
// protocol-compatible values so SimHub never waits for an unlock.
#ifndef SH14_UPLOAD_PROTECTION
#define SH14_UPLOAD_PROTECTION 0
#endif
#ifndef SH14_UPLOAD_DELAY
#define SH14_UPLOAD_DELAY 15000
#endif

// LED strips: total strip LED count (0 = none, matrix only)
#ifndef SH14_STRIP_RGBLEDCOUNT
#define SH14_STRIP_RGBLEDCOUNT 0
#endif
#ifndef SH14_LEDS_LAYOUT
#define SH14_LEDS_LAYOUT ""
#endif
#ifndef SH14_DEFAULT_BUTTONS_COLORS
#define SH14_DEFAULT_BUTTONS_COLORS ""
#endif

// 8x8 matrix: 1 = present, 0 = absent
#ifndef SH14_MATRIX_ENABLED
#define SH14_MATRIX_ENABLED 1
#endif
// Device side remapping, only needed when the panel is mounted rotated or wired
// differently from the classic protocol's expectations. When left at 0 the
// callback receives the same index order the classic protocol uses, so both
// protocols light the panel identically.
// rotation: 0 / 90 / 180 / 270 ; layout: 0 left-to-right, 1 serpentine,
// 2 reverse-serpentine
#ifndef SH14_MATRIX_ROTATION
#define SH14_MATRIX_ROTATION 0
#endif
#ifndef SH14_MATRIX_LAYOUT
#define SH14_MATRIX_LAYOUT 0
#endif
// Per-LED brightness scale applied before the callback (255 = raw values)
#ifndef SH14_MATRIX_MAX_BRIGHTNESS
#define SH14_MATRIX_MAX_BRIGHTNESS 255
#endif

// Timeout for the bytes that follow a detected command. At 115200 baud a byte
// takes ~87us, so this only ever triggers on a truncated transmission, where
// blocking forever (what the reference driver does) would hang the loop.
#ifndef SH14_READ_TIMEOUT_MS
#define SH14_READ_TIMEOUT_MS 250
#endif

// ------------------------- callbacks (implement these) -----------------------

// 64*3 bytes, row-major (index = row*8+col), already rotation/layout mapped and
// brightness scaled. Implement it in main.cpp, pushing the frame to the panel.
void sh14OnMatrixFrame(const uint8_t* rgb64x3);

#if SH14_STRIP_RGBLEDCOUNT > 0
// SH14_STRIP_RGBLEDCOUNT*3 bytes in strip order
void sh14OnLedsFrame(const uint8_t* rgb);
#endif

// ------------------------- implementation ------------------------------------

namespace sh14 {

inline Stream& port() { return Serial; }

inline int waitByte() {
  uint32_t start = millis();
  while (!port().available()) {
    if (millis() - start > SH14_READ_TIMEOUT_MS) return -1;
    // Yield instead of delay(): a 195 byte matrix frame must not cost a
    // millisecond per byte, the bytes are already on their way.
    yield();
  }
  return port().read();
}

inline bool readValidationSequence() {
  bool valid = true;
  for (int i = 0; i < 3; i++) {
    int b = waitByte();
    if (b != (0xFF - i)) valid = false;
  }
  return valid;
}

inline int remapIndex(int index, int rotation) {
  int row = index / 8, col = index % 8;
  switch (rotation) {
    case 90:  return col * 8 + (7 - row);
    case 180: return (7 - row) * 8 + (7 - col);
    case 270: return (7 - col) * 8 + row;
    default:  return index;
  }
}

inline void readMatrixCommand() {
  static uint8_t rgb[64 * 3];
  for (int i = 0; i < 64; i++) {
    int r = waitByte();
    int g = waitByte();
    int b = waitByte();
    if (r < 0 || g < 0 || b < 0) return;  // truncated frame: drop it

    int n = remapIndex(i, SH14_MATRIX_ROTATION);
    if (SH14_MATRIX_LAYOUT == 1 || SH14_MATRIX_LAYOUT == 2) {
      int Y = n / 8, X = n % 8;
      if ((SH14_MATRIX_LAYOUT == 1 && !(Y % 2)) ||
          (SH14_MATRIX_LAYOUT == 2 && (Y % 2))) X = 7 - X;
      n = Y * 8 + X;
    }
    rgb[n * 3 + 0] = (uint8_t)(r * SH14_MATRIX_MAX_BRIGHTNESS / 255);
    rgb[n * 3 + 1] = (uint8_t)(g * SH14_MATRIX_MAX_BRIGHTNESS / 255);
    rgb[n * 3 + 2] = (uint8_t)(b * SH14_MATRIX_MAX_BRIGHTNESS / 255);
  }
  if (readValidationSequence()) sh14OnMatrixFrame(rgb);
}

#if SH14_STRIP_RGBLEDCOUNT > 0
inline void readLedsCommand() {
  static uint8_t rgb[SH14_STRIP_RGBLEDCOUNT * 3];
  for (int i = 0; i < SH14_STRIP_RGBLEDCOUNT; i++) {
    int r = waitByte();
    int g = waitByte();
    int b = waitByte();
    if (r < 0 || g < 0 || b < 0) return;
    rgb[i * 3 + 0] = (uint8_t)r;
    rgb[i * 3 + 1] = (uint8_t)g;
    rgb[i * 3 + 2] = (uint8_t)b;
  }
  if (readValidationSequence()) sh14OnLedsFrame(rgb);
}
#endif

// The classic protocol blanks its outputs after 5s of silence (main.cpp ->
// Command_Shutdown()). The standard protocol has no such timeout -- the
// reference driver simply holds the last frame -- and SimHub is free to go
// quiet between updates. Without this flag the panel is lit by every smatx
// frame and immediately blanked by the next loop() iteration: it flickers.
inline bool& everActive() {
  static bool value = false;
  return value;
}

inline void dispatch(const char* cmd) {
  everActive() = true;
  if (!strcmp(cmd, "proto"))      port().println("SIMHUB_1.4");
  else if (!strcmp(cmd, "dname")) port().println(SH14_DEVICE_NAME);
  else if (!strcmp(cmd, "bname")) port().println(SH14_DEVICE_BRAND);
  else if (!strcmp(cmd, "dpict")) port().println(SH14_DEVICE_PICTURE_URL);
  else if (!strcmp(cmd, "ddete")) port().println((int)SH14_DEVICE_AUTODETECT_ALLOWED);
  else if (!strcmp(cmd, "sdete")) port().println((int)SH14_SCREEN_AND_LEDS_RELATION);
  else if (!strcmp(cmd, "dtype")) port().println((int)SH14_DEVICE_TYPE);
  else if (!strcmp(cmd, "ulock")) { port().print((int)SH14_UPLOAD_PROTECTION); port().print(','); port().println((int)SH14_UPLOAD_DELAY); }
  else if (!strcmp(cmd, "snumb")) port().println(getUniqueId());
  else if (!strcmp(cmd, "rnumb")) port().println(getUniqueId());  // MAC cannot be reset
  else if (!strcmp(cmd, "ledsc")) port().println((int)SH14_STRIP_RGBLEDCOUNT);
  else if (!strcmp(cmd, "ledsl")) port().println(SH14_LEDS_LAYOUT);
  else if (!strcmp(cmd, "butdc")) port().println(SH14_DEFAULT_BUTTONS_COLORS);
#if SH14_STRIP_RGBLEDCOUNT > 0
  else if (!strcmp(cmd, "sleds")) readLedsCommand();
#endif
  else if (!strcmp(cmd, "matxc")) port().println(SH14_MATRIX_ENABLED ? 1 : 0);
  else if (!strcmp(cmd, "smatx")) readMatrixCommand();
  else if (!strcmp(cmd, "fansc")) port().println(0);
  else if (!strcmp(cmd, "motoc")) port().println(0);
  else if (!strcmp(cmd, "sfans")) { /* no fans: payload only sent when count > 0 */ }
  else if (!strcmp(cmd, "smoto")) { /* no motors */ }
  else if (!strcmp(cmd, "vendo")) { int hi = waitByte(); int lo = waitByte(); if (hi >= 0 && lo >= 0) { int n = (hi << 8) + lo; for (int i = 0; i < n; i++) waitByte(); } }
  else if (!strcmp(cmd, "unloc")) port().println("Upload unlocked");
  // Unknown command: stay silent, the host retries.
}

}  // namespace sh14

// The classic protocol blanks its outputs after 5s of silence (main.cpp ->
// Command_Shutdown()). SIMHUB_1.4 has no such timeout -- the reference driver
// simply holds the last frame -- so once this device has been driven over the
// standard protocol that shutdown must stay away. Without it every frame is
// blanked by the next loop() iteration and the panel flickers.
inline bool sh14BlocksIdleShutdown() { return sh14::everActive(); }

// Feed one byte read from the serial link.
// Returns true when the byte was consumed by the SIMHUB_1.4 handler; the caller
// must then skip its own protocol handling for this byte.
inline bool sh14FeedByte(uint8_t c) {
  static int ffRun = 0;

  // Bytes of a 0xFF run are passed through: the classic parser ignores anything
  // that is not 0x01, so they are harmless there.
  if (ffRun < 6) {
    if (c == 0xFF) { ffRun++; return false; }
    ffRun = 0;
  }

  if (ffRun >= 6 && c != 0xFF) {
    // Bright classic RGB traffic (two adjacent white pixels) also produces runs
    // of 0xFF. Require the byte to be an initial that actually starts a standard
    // protocol command before swallowing anything; everything else is handed
    // back to the ARQ parser untouched.
    // strchr() would match the terminating NUL, hence the explicit c == 0 test.
    if (c == 0 || c == 0xFF || !strchr("pdbusrlmfv", (char)c)) {
      ffRun = 0;
      return false;
    }

    char cmd[6] = {0};
    cmd[0] = (char)c;
    for (int i = 1; i < 5; i++) {
      int b = sh14::waitByte();
      if (b < 0) { ffRun = 0; return true; }  // truncated command: swallow it
      cmd[i] = (char)b;
    }
    ffRun = 0;

    sh14::dispatch(cmd);
    return true;
  }
  return false;
}

#else  // !ENABLE_SIMHUB14

// Disabled: consume nothing, so the classic parser sees every byte as before,
// and never block the classic idle shutdown.
inline bool sh14FeedByte(uint8_t) { return false; }
inline bool sh14BlocksIdleShutdown() { return false; }

#endif  // ENABLE_SIMHUB14
