#!/usr/bin/env node
/**
 * ESP-SimHub Serial Device Simulator
 *
 * Simulates the full ARQ transport + SimHub application-layer handshake and
 * feature enumeration so you can verify the host (SimHub) side of the
 * protocol without a physical ESP device.
 *
 * Usage:
 *   node simulator.js [serialPort] [baudRate]
 *
 *   serialPort  – defaults to the first argument or /dev/ttyUSB0
 *   baudRate    – defaults to 19200
 *
 * Virtual serial pair (Linux/macOS):
 *   socat -d -d pty,raw,echo=0 pty,raw,echo=0
 *   # socat will print two pty paths, e.g. /dev/pts/3 and /dev/pts/4
 *   # Point SimHub at one end and run this simulator against the other.
 *   node simulator.js /dev/pts/3
 *
 * Protocol reference: ../PROTOCOL.md
 */

'use strict';

const { SerialPort } = require('serialport');

// ─────────────────────────────────────────────────────────────────────────────
// Configuration – tweak to match whatever features you want to advertise
// ─────────────────────────────────────────────────────────────────────────────
const CONFIG = {
  deviceName:    'ESP-SimHub-Sim',
  uniqueId:      'AA:BB:CC:DD:EE:FF',
  firmwareVersion: 0x6A,              // 'j' – same as real firmware
  // MCU signature – pretends to be an Arduino Mega ATmega2560
  mcuSignature:  [0x1E, 0x98, 0x01],
  rgbLedCount:   8,
  tm1638Count:   0,
  simpleModules: 0,
  buttonCount:   0,
  // Feature flags advertised to SimHub (always include G N I J P X)
  features: ['G', 'N', 'I', 'J', 'P', 'X'],
  // Extended sub-commands advertised in the 'X list' response
  extendedCommands: ['mcutype', 'keepalive'],
};

// ─────────────────────────────────────────────────────────────────────────────
// CRC-8 table (matches the firmware's crc_table_crc8 in ArqSerial.h)
// ─────────────────────────────────────────────────────────────────────────────
const CRC8_TABLE = Buffer.from([
    0,213,127,170,254, 43,129, 84, 41,252, 86,131,215,  2,168,125,
   82,135, 45,248,172,121,211,  6,123,174,  4,209,133, 80,250, 47,
  164,113,219, 14, 90,143, 37,240,141, 88,242, 39,115,166, 12,217,
  246, 35,137, 92,  8,221,119,162,223, 10,160,117, 33,244, 94,139,
  157, 72,226, 55, 99,182, 28,201,180, 97,203, 30, 74,159, 53,224,
  207, 26,176,101, 49,228, 78,155,230, 51,153, 76, 24,205,103,178,
   57,236, 70,147,199, 18,184,109, 16,197,111,186,238, 59,145, 68,
  107,190, 20,193,149, 64,234, 63, 66,151, 61,232,188,105,195, 22,
  239, 58,144, 69, 17,196,110,187,198, 19,185,108, 56,237, 71,146,
  189,104,194, 23, 67,150, 60,233,148, 65,235, 62,106,191, 21,192,
   75,158, 52,225,181, 96,202, 31, 98,183, 29,200,156, 73,227, 54,
   25,204,102,179,231, 50,152, 77, 48,229, 79,154,206, 27,177,100,
  114,167, 13,216,140, 89,243, 38, 91,142, 36,241,165,112,218, 15,
   32,245, 95,138,222, 11,161,116,  9,220,118,163,247, 34,136, 93,
  214,  3,169,124, 40,253, 87,130,255, 42,128, 85,  1,212,126,171,
  132, 81,251, 46,122,175,  5,208,173,120,210,  7, 83,134, 44,249,
]);

function crc8(packetId, length, data) {
  let crc = 0;
  crc = CRC8_TABLE[crc ^ packetId];
  crc = CRC8_TABLE[crc ^ length];
  for (let i = 0; i < data.length; i++) {
    crc = CRC8_TABLE[crc ^ data[i]];
  }
  return crc;
}

// ─────────────────────────────────────────────────────────────────────────────
// Debug logging helpers
// ─────────────────────────────────────────────────────────────────────────────
const LOG_LEVEL = { TRACE: 0, DEBUG: 1, INFO: 2, WARN: 3, ERROR: 4 };
const currentLogLevel = LOG_LEVEL.TRACE;

function timestamp() {
  return new Date().toISOString().replace('T', ' ').replace('Z', '');
}

function log(level, tag, msg) {
  if (level >= currentLogLevel) {
    const levelName = Object.keys(LOG_LEVEL).find(k => LOG_LEVEL[k] === level);
    console.log(`[${timestamp()}] [${levelName.padEnd(5)}] [${tag.padEnd(8)}] ${msg}`);
  }
}

function hexDump(buf, label) {
  if (!Buffer.isBuffer(buf)) buf = Buffer.from(buf);
  const hex = [...buf].map(b => b.toString(16).padStart(2, '0').toUpperCase()).join(' ');
  const ascii = [...buf].map(b => (b >= 0x20 && b < 0x7F) ? String.fromCharCode(b) : '.').join('');
  log(LOG_LEVEL.TRACE, 'HEX', `${label}: [${hex}]  "${ascii}"`);
}

// ─────────────────────────────────────────────────────────────────────────────
// Output frame builders  (device → SimHub)
// ─────────────────────────────────────────────────────────────────────────────

/** ACK: 03 <packetId> */
function makeAck(packetId) {
  return Buffer.from([0x03, packetId]);
}

/** NACK: 04 <lastValidId> <reason> */
function makeNack(lastValidId, reason) {
  return Buffer.from([0x04, lastValidId, reason]);
}

/** Single-byte output: 08 <byte> */
function makeSingleByte(value) {
  return Buffer.from([0x08, value & 0xFF]);
}

/**
 * String frame: 06 <length> <...bytes> 20
 * The firmware does NOT add a null terminator – length is the raw string length.
 */
function makeStringFrame(str) {
  const strBuf = Buffer.isBuffer(str) ? str : Buffer.from(str, 'latin1');
  const frame = Buffer.allocUnsafe(3 + strBuf.length);
  frame[0] = 0x06;
  frame[1] = strBuf.length;
  strBuf.copy(frame, 2);
  frame[2 + strBuf.length] = 0x20;
  return frame;
}

/**
 * String frame with trailing newline: 06 <length+1> <...bytes> 0A 20
 * Mirrors firmware PrintLn().
 */
function makeStringFrameLn(str) {
  return makeStringFrame(str + '\n');
}

/** Debug frame: 07 <length+1> <...bytes> 0A 20  (mirrors DebugPrintLn) */
function makeDebugFrame(str) {
  const strBuf = Buffer.from(str + '\n', 'latin1');
  const frame = Buffer.allocUnsafe(3 + strBuf.length);
  frame[0] = 0x07;
  frame[1] = strBuf.length;
  strBuf.copy(frame, 2);
  frame[2 + strBuf.length] = 0x20;
  return frame;
}

// ─────────────────────────────────────────────────────────────────────────────
// Simulator state
// ─────────────────────────────────────────────────────────────────────────────
const state = {
  lastValidPacketId: 255,   // starts at 0xFF so first expected is 0x00
  rxBuffer: Buffer.alloc(0),
  sessionActive: false,
  keepAliveCount: 0,
  baudRate: 19200,
};

// ─────────────────────────────────────────────────────────────────────────────
// ARQ packet parser
// ─────────────────────────────────────────────────────────────────────────────

/** Expected next packet ID – wraps 0→127→0, and 0xFF is always valid (broadcast) */
function expectedPacketId() {
  if (state.lastValidPacketId > 127) return 0;
  return (state.lastValidPacketId + 1) & 0xFF;
}

/**
 * Try to consume one complete ARQ packet from state.rxBuffer.
 * Returns { packetId, payload } on success, or null if more data is needed,
 * or throws a structured error object { reason, lastValid } on protocol error.
 */
function tryParsePacket() {
  const buf = state.rxBuffer;

  // Minimum packet: header(2) + id(1) + length(1) + data(1) + crc(1) = 6 bytes
  if (buf.length < 6) return null;

  // Find 01 01 header
  if (buf[0] !== 0x01 || buf[1] !== 0x01) {
    // Discard bytes until we find the header or exhaust the buffer
    let i = 1;
    while (i < buf.length - 1 && !(buf[i] === 0x01 && buf[i + 1] === 0x01)) i++;
    log(LOG_LEVEL.WARN, 'ARQ', `Discarding ${i} non-header byte(s): ${buf.slice(0, i).toString('hex')}`);
    state.rxBuffer = buf.slice(i);
    return null;
  }

  const packetId = buf[2];
  const length   = buf[3];

  // Validate length
  if (length === 0 || length > 32) {
    log(LOG_LEVEL.WARN, 'ARQ', `Bad length ${length} (must be 1-32), sending NACK reason 0x02`);
    state.rxBuffer = buf.slice(4); // discard header+id+length
    throw { reason: 0x02, lastValid: state.lastValidPacketId };
  }

  // Need header(2) + id(1) + length(1) + data(length) + crc(1)
  const totalNeeded = 4 + length + 1;
  if (buf.length < totalNeeded) return null; // wait for more data

  const payload = buf.slice(4, 4 + length);
  const receivedCrc = buf[4 + length];

  // Validate CRC
  const expectedCrc = crc8(packetId, length, payload);
  if (receivedCrc !== expectedCrc) {
    log(LOG_LEVEL.WARN, 'ARQ',
      `CRC mismatch – got 0x${receivedCrc.toString(16).padStart(2,'0')}, ` +
      `expected 0x${expectedCrc.toString(16).padStart(2,'0')} – sending NACK reason 0x04`);
    state.rxBuffer = buf.slice(totalNeeded);
    throw { reason: 0x04, lastValid: state.lastValidPacketId };
  }

  // Validate sequence number
  const expected = expectedPacketId();
  if (packetId !== 0xFF && packetId !== expected) {
    log(LOG_LEVEL.WARN, 'ARQ',
      `Out-of-sequence packet – got ${packetId}, expected ${expected} – sending NACK reason 0x01`);
    state.rxBuffer = buf.slice(totalNeeded);
    throw { reason: 0x01, lastValid: state.lastValidPacketId };
  }

  // All good – consume the bytes and update state
  state.rxBuffer = buf.slice(totalNeeded);
  state.lastValidPacketId = packetId;

  hexDump(buf.slice(0, totalNeeded), `  ARQ IN  [id=0x${packetId.toString(16).padStart(2,'0')}]`);
  return { packetId, payload };
}

// ─────────────────────────────────────────────────────────────────────────────
// Command handlers  (device-side responses)
// ─────────────────────────────────────────────────────────────────────────────

/**
 * Handlers return a Buffer (or array of Buffers) to write, or null for silence.
 * The payload passed in already has the 0x03 header stripped.
 *
 * Application layer data layout (inside the ARQ payload):
 *   payload[0]  = 0x03  MESSAGE_HEADER  (stripped before calling handlers)
 *   payload[1]  = command char
 *   payload[2+] = optional data bytes
 */

function handleHello(extra) {
  log(LOG_LEVEL.INFO, 'CMD', `Hello ('1') – responding with firmware version 0x${CONFIG.firmwareVersion.toString(16)}`);
  // The firmware reads one more byte (the 0x10 end-of-line) via FlowSerialTimedRead()
  // before replying.  That byte is already in `extra` if present – no issue.
  return makeSingleByte(CONFIG.firmwareVersion);
}

function handleFeatures() {
  log(LOG_LEVEL.INFO, 'CMD', `Features ('0') – advertising: ${CONFIG.features.join(', ')}`);
  const frames = [];
  for (const f of CONFIG.features) {
    frames.push(makeStringFrame(f));
    log(LOG_LEVEL.DEBUG, 'CMD', `  → feature '${f}'`);
  }
  // Terminator: newline character as a single-char string frame
  frames.push(makeStringFrame('\n'));
  log(LOG_LEVEL.DEBUG, 'CMD', `  → feature terminator '\\n'`);
  return Buffer.concat(frames);
}

function handleRgbLedCount() {
  log(LOG_LEVEL.INFO, 'CMD', `RGB LED count ('4') – reporting ${CONFIG.rgbLedCount}`);
  return makeSingleByte(CONFIG.rgbLedCount);
}

function handleTm1638Count() {
  log(LOG_LEVEL.INFO, 'CMD', `TM1638 count ('2') – reporting ${CONFIG.tm1638Count}`);
  return makeSingleByte(CONFIG.tm1638Count);
}

function handleSimpleModulesCount() {
  log(LOG_LEVEL.INFO, 'CMD', `Simple modules count ('B') – reporting ${CONFIG.simpleModules}`);
  return makeSingleByte(CONFIG.simpleModules);
}

function handleDeviceName() {
  log(LOG_LEVEL.INFO, 'CMD', `Device name ('N') – "${CONFIG.deviceName}"`);
  return makeStringFrameLn(CONFIG.deviceName);
}

function handleUniqueId() {
  log(LOG_LEVEL.INFO, 'CMD', `Unique ID ('I') – "${CONFIG.uniqueId}"`);
  return makeStringFrameLn(CONFIG.uniqueId);
}

function handleButtonCount() {
  log(LOG_LEVEL.INFO, 'CMD', `Button count ('J') – reporting ${CONFIG.buttonCount}`);
  return makeSingleByte(CONFIG.buttonCount);
}

function handleSetBaudrate(extra) {
  const code = extra[0];
  const BAUD_MAP = {
    1: 300, 2: 1200, 3: 2400, 4: 4800, 5: 9600, 6: 14400,
    7: 19200, 8: 28800, 9: 38400, 10: 57600, 11: 115200,
    12: 230400, 13: 250000, 14: 1000000, 15: 2000000, 16: 200000, 17: 500000,
  };
  const newBaud = BAUD_MAP[code];
  if (newBaud) {
    log(LOG_LEVEL.INFO, 'CMD', `Set baud rate ('8') – code ${code} → ${newBaud}`);
    state.baudRate = newBaud;
    // NOTE: a real device would call Serial.begin(newBaud) after a 200ms delay.
    // The simulator stays at the current port baud – log and continue.
    log(LOG_LEVEL.WARN, 'CMD', `  ⚠ Simulator does NOT actually change port baud rate.`);
  } else {
    log(LOG_LEVEL.WARN, 'CMD', `Set baud rate ('8') – unknown code ${code}, ignoring`);
  }
  return null; // no response
}

function handleGear(extra) {
  const gear = extra[0] ? String.fromCharCode(extra[0]) : '?';
  log(LOG_LEVEL.DEBUG, 'CMD', `Gear ('G') – current gear: '${gear}'`);
  return null; // no response
}

function handleRgbLedData() {
  log(LOG_LEVEL.DEBUG, 'CMD', `RGB LED data ('6') – consumed, sending 0x15 ACQ`);
  return makeSingleByte(0x15);
}

function handleRgbMatrixData() {
  log(LOG_LEVEL.DEBUG, 'CMD', `RGB matrix data ('R') – consumed, sending 0x15 ACQ`);
  return makeSingleByte(0x15);
}

function handleCustomProtocol() {
  log(LOG_LEVEL.DEBUG, 'CMD', `Custom protocol ('P') – consumed, sending 0x15 ACQ`);
  return makeSingleByte(0x15);
}

function handleTm1638Data() {
  log(LOG_LEVEL.DEBUG, 'CMD', `TM1638 data ('3') – consumed (no response)`);
  return null;
}

function handleSevenSegData() {
  log(LOG_LEVEL.DEBUG, 'CMD', `7-segment data ('S') – consumed (no response)`);
  return null;
}

function handleMotors(extra) {
  const action = extra[0] ? String.fromCharCode(extra[0]) : '?';
  log(LOG_LEVEL.DEBUG, 'CMD', `Motors ('V') – action '${action}'`);
  if (action === 'C') {
    log(LOG_LEVEL.INFO, 'CMD', `  Motor count query – reporting 0 motors`);
    // Format: 0xFF (sentinel) + count byte + provider names... + newline
    const countBuf   = Buffer.from([0xFF, 0x00]); // 0 motors
    const newlineBuf = makeStringFrameLn('');       // empty line terminator
    return Buffer.concat([countBuf, newlineBuf]);
  }
  if (action === 'S') {
    log(LOG_LEVEL.DEBUG, 'CMD', `  Motor set – no motors to update`);
    return null;
  }
  log(LOG_LEVEL.WARN, 'CMD', `  Unknown motor action '${action}'`);
  return null;
}

// Extended command dispatcher – payload after 'X ' prefix
function handleExtendedCommand(subCmd, extra) {
  log(LOG_LEVEL.DEBUG, 'CMD', `Extended ('X') sub-command: "${subCmd}"`);

  if (subCmd === 'list') {
    log(LOG_LEVEL.INFO, 'CMD', `  Extended command list – advertising: ${CONFIG.extendedCommands.join(', ')}`);
    const frames = CONFIG.extendedCommands.map(c => makeStringFrameLn(c));
    // Terminator: empty PrintLn (just a newline single-byte output)
    frames.push(makeSingleByte('\n'.charCodeAt(0)));
    return Buffer.concat(frames);
  }

  if (subCmd === 'mcutype') {
    const [s0, s1, s2] = CONFIG.mcuSignature;
    log(LOG_LEVEL.INFO, 'CMD',
      `  MCU type – signature 0x${s0.toString(16)} 0x${s1.toString(16)} 0x${s2.toString(16)}`);
    // Firmware sends three raw bytes via FlowSerialPrint (each wrapped in 0x08)
    return Buffer.concat([
      makeSingleByte(s0),
      makeSingleByte(s1),
      makeSingleByte(s2),
    ]);
  }

  if (subCmd === 'keepalive') {
    state.keepAliveCount++;
    log(LOG_LEVEL.TRACE, 'CMD', `  Keep-alive #${state.keepAliveCount}`);
    return null; // no response
  }

  // Gauge sub-commands – just consume and ignore
  const gaugeCommands = ['tach', 'tachometer', 'speedo', 'boost', 'boostgauge',
                         'temp', 'tempgauge', 'fuel', 'fuelgauge', 'cons',
                         'consumptiongauge', 'encoders'];
  if (gaugeCommands.includes(subCmd)) {
    log(LOG_LEVEL.TRACE, 'CMD', `  Gauge/encoder data for "${subCmd}" – silently consumed`);
    return null;
  }

  log(LOG_LEVEL.WARN, 'CMD', `  Unknown extended command: "${subCmd}"`);
  return null;
}

// ─────────────────────────────────────────────────────────────────────────────
// Main packet dispatch
// ─────────────────────────────────────────────────────────────────────────────

/**
 * Decode the ARQ payload (starting at byte 0 = 0x03 MESSAGE_HEADER) and
 * return the bytes to send back (after the ACK), or null.
 */
function dispatchPayload(payload) {
  if (payload[0] !== 0x03) {
    log(LOG_LEVEL.WARN, 'DISPATCH', `Expected MESSAGE_HEADER 0x03, got 0x${payload[0].toString(16)} – ignoring`);
    return null;
  }

  const cmd   = String.fromCharCode(payload[1]);
  const extra = payload.slice(2); // everything after the command byte

  log(LOG_LEVEL.DEBUG, 'DISPATCH', `Command '${cmd}' (0x${payload[1].toString(16).padStart(2,'0')}) extra=[${[...extra].map(b=>b.toString(16).padStart(2,'0')).join(' ')}]`);

  switch (cmd) {
    case '1': return handleHello(extra);          // Hello
    case '0': return handleFeatures();            // Features
    case '4': return handleRgbLedCount();         // RGB count
    case '2': return handleTm1638Count();         // TM1638 count
    case 'B': return handleSimpleModulesCount();  // Simple modules count
    case 'N': return handleDeviceName();          // Device name
    case 'I': return handleUniqueId();            // Unique ID
    case 'J': return handleButtonCount();         // Button count
    case '8': return handleSetBaudrate(extra);    // Set baud rate
    case 'G': return handleGear(extra);           // Gear display
    case '6': return handleRgbLedData();          // RGB LED data
    case 'R': return handleRgbMatrixData();       // RGB matrix data
    case 'P': return handleCustomProtocol();      // Custom protocol
    case '3': return handleTm1638Data();          // TM1638 data
    case 'S': return handleSevenSegData();        // 7-segment data
    case 'V': return handleMotors(extra);         // Motors
    case 'X': {
      // Extended command – sub-command is a space/newline-terminated ASCII string
      // that starts at extra[0].  It may be followed by data.
      const rawSubCmd = Buffer.concat([extra]);
      // Read until 0x20 (space) or 0x0A (newline) or end of buffer
      let end = rawSubCmd.length;
      for (let i = 0; i < rawSubCmd.length; i++) {
        if (rawSubCmd[i] === 0x20 || rawSubCmd[i] === 0x0A) { end = i; break; }
      }
      const subCmd   = rawSubCmd.slice(0, end).toString('ascii').trim();
      const subExtra = rawSubCmd.slice(end + 1);
      return handleExtendedCommand(subCmd, subExtra);
    }
    default:
      log(LOG_LEVEL.WARN, 'DISPATCH', `Unknown command '${cmd}' (0x${payload[1].toString(16).padStart(2,'0')}) – ignoring`);
      return null;
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// Serial I/O
// ─────────────────────────────────────────────────────────────────────────────

function writeToPort(port, data, label) {
  if (!data || data.length === 0) return;
  hexDump(data, `  ARQ OUT [${label}]`);
  port.write(data, (err) => {
    if (err) log(LOG_LEVEL.ERROR, 'SERIAL', `Write error: ${err.message}`);
  });
}

function onData(port, rawData) {
  state.rxBuffer = Buffer.concat([state.rxBuffer, rawData]);
  log(LOG_LEVEL.TRACE, 'SERIAL', `Received ${rawData.length} byte(s), buffer now ${state.rxBuffer.length} byte(s)`);

  let keepParsing = true;
  while (keepParsing && state.rxBuffer.length > 0) {
    keepParsing = false;

    let packet;
    try {
      packet = tryParsePacket();
    } catch (e) {
      // Protocol error – send NACK
      const nack = makeNack(e.lastValid, e.reason);
      log(LOG_LEVEL.WARN, 'ARQ', `Sending NACK [lastValid=${e.lastValid}, reason=0x${e.reason.toString(16)}]`);
      writeToPort(port, nack, `NACK reason=0x${e.reason.toString(16)}`);
      keepParsing = true;
      continue;
    }

    if (!packet) break; // need more data

    keepParsing = true;

    const { packetId, payload } = packet;
    log(LOG_LEVEL.INFO, 'ARQ',
      `Valid packet id=0x${packetId.toString(16).padStart(2,'0')} ` +
      `length=${payload.length} ` +
      `payload=[${[...payload].map(b=>b.toString(16).padStart(2,'0')).join(' ')}]`);

    // Always ACK first
    const ack = makeAck(packetId);
    writeToPort(port, ack, `ACK id=0x${packetId.toString(16).padStart(2,'0')}`);

    // Then handle the command
    const response = dispatchPayload(payload);
    if (response) {
      writeToPort(port, response, `response cmd='${String.fromCharCode(payload[1])}'`);
    }
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// Entry point
// ─────────────────────────────────────────────────────────────────────────────

const portPath = process.argv[2] || '/dev/ttyUSB0';
const baudRate = parseInt(process.argv[3] || '19200', 10);

log(LOG_LEVEL.INFO, 'MAIN', '═══════════════════════════════════════════════════');
log(LOG_LEVEL.INFO, 'MAIN', ' ESP-SimHub Serial Device Simulator');
log(LOG_LEVEL.INFO, 'MAIN', '═══════════════════════════════════════════════════');
log(LOG_LEVEL.INFO, 'MAIN', `Port:        ${portPath}`);
log(LOG_LEVEL.INFO, 'MAIN', `Baud rate:   ${baudRate}`);
log(LOG_LEVEL.INFO, 'MAIN', `Device name: ${CONFIG.deviceName}`);
log(LOG_LEVEL.INFO, 'MAIN', `Unique ID:   ${CONFIG.uniqueId}`);
log(LOG_LEVEL.INFO, 'MAIN', `Version:     0x${CONFIG.firmwareVersion.toString(16)} ('${String.fromCharCode(CONFIG.firmwareVersion)}')`);
log(LOG_LEVEL.INFO, 'MAIN', `Features:    ${CONFIG.features.join(', ')}`);
log(LOG_LEVEL.INFO, 'MAIN', `RGB LEDs:    ${CONFIG.rgbLedCount}`);
log(LOG_LEVEL.INFO, 'MAIN', '───────────────────────────────────────────────────');
log(LOG_LEVEL.INFO, 'MAIN', 'Waiting for SimHub to connect…');

const port = new SerialPort({ path: portPath, baudRate }, (err) => {
  if (err) {
    log(LOG_LEVEL.ERROR, 'MAIN', `Failed to open port: ${err.message}`);
    log(LOG_LEVEL.INFO,  'MAIN', `Tip: create a virtual pair with:  socat -d -d pty,raw,echo=0 pty,raw,echo=0`);
    process.exit(1);
  }
  log(LOG_LEVEL.INFO, 'MAIN', `Port ${portPath} opened at ${baudRate} baud`);
});

port.on('data', (data) => onData(port, data));

port.on('error', (err) => {
  log(LOG_LEVEL.ERROR, 'SERIAL', `Port error: ${err.message}`);
});

port.on('close', () => {
  log(LOG_LEVEL.INFO, 'SERIAL', 'Port closed');
  process.exit(0);
});

process.on('SIGINT', () => {
  log(LOG_LEVEL.INFO, 'MAIN', 'Shutting down…');
  port.close();
});
