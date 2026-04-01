#!/usr/bin/env node
/**
 * ESP-SimHub Host Simulator
 *
 * Acts as the SimHub host side of the protocol, driving the full handshake
 * and feature enumeration against a REAL ESP device over serial.  Use this
 * to verify that your firmware is behaving correctly without needing SimHub.
 *
 * Usage:
 *   node simulator.js <serialPort> [baudRate]
 *
 *   serialPort  – e.g. /dev/ttyUSB0  or  COM3
 *   baudRate    – default 19200
 *
 * What it does:
 *   Phase 1  – Hello (broadcast packet, expects version byte back)
 *   Phase 2  – Feature enumeration (features, counts, name, ID, …)
 *   Phase 3  – Extended command list  (X list, X mcutype)
 *   Phase 4  – Continuous keepalive heartbeats every 500 ms
 *              (press Ctrl-C to stop)
 *
 * Protocol reference: ../PROTOCOL.md
 */

'use strict';

const { SerialPort } = require('serialport');

// ─────────────────────────────────────────────────────────────────────────────
// CLI args
// ─────────────────────────────────────────────────────────────────────────────
const portPath = process.argv[2];
const baudRate = parseInt(process.argv[3] || '19200', 10);

if (!portPath) {
  console.error('Usage: node simulator.js <serialPort> [baudRate]');
  console.error('  e.g. node simulator.js /dev/ttyUSB0 19200');
  process.exit(1);
}

// ─────────────────────────────────────────────────────────────────────────────
// CRC-8 table – exact copy from ArqSerial.h crc_table_crc8[]
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
  for (const b of data) crc = CRC8_TABLE[crc ^ b];
  return crc;
}

// ─────────────────────────────────────────────────────────────────────────────
// Logging
// ─────────────────────────────────────────────────────────────────────────────
function ts() { return new Date().toISOString().slice(11, 23); }

function log(level, tag, msg) {
  console.log(`[${ts()}] ${level.padEnd(5)} [${tag.padEnd(10)}] ${msg}`);
}

function hexStr(buf) {
  return [...buf].map(b => b.toString(16).padStart(2, '0').toUpperCase()).join(' ');
}

function asciiStr(buf) {
  return [...buf].map(b => (b >= 0x20 && b < 0x7F) ? String.fromCharCode(b) : '.').join('');
}

function logHex(dir, label, buf) {
  if (!Buffer.isBuffer(buf)) buf = Buffer.from(buf);
  log('TRACE', dir, `${label}: [${hexStr(buf)}]  "${asciiStr(buf)}"`);
}

// ─────────────────────────────────────────────────────────────────────────────
// ARQ packet builder  (host → device)
//   01 01 <packetId> <length> <data…> <crc8>
// ─────────────────────────────────────────────────────────────────────────────
function buildArqPacket(packetId, data) {
  const length = data.length;
  const checksum = crc8(packetId, length, data);
  const pkt = Buffer.allocUnsafe(4 + length + 1);
  pkt[0] = 0x01; pkt[1] = 0x01;
  pkt[2] = packetId;
  pkt[3] = length;
  data.copy(pkt, 4);
  pkt[4 + length] = checksum;
  return pkt;
}

/**
 * Build a SimHub command payload.
 * Application-layer format: 0x03 <cmdChar> [extra bytes…]
 */
function buildPayload(cmdChar, extra) {
  const extraBuf = extra ? (Buffer.isBuffer(extra) ? extra : Buffer.from(extra)) : Buffer.alloc(0);
  const buf = Buffer.allocUnsafe(2 + extraBuf.length);
  buf[0] = 0x03;              // MESSAGE_HEADER
  buf[1] = cmdChar.charCodeAt(0);
  extraBuf.copy(buf, 2);
  return buf;
}

// ─────────────────────────────────────────────────────────────────────────────
// Host-side sequence state
// ─────────────────────────────────────────────────────────────────────────────
let nextPacketId = 0;           // increments 0→127→0 for normal packets
const BROADCAST_ID = 0xFF;

function allocPacketId(broadcast = false) {
  if (broadcast) return BROADCAST_ID;
  const id = nextPacketId;
  nextPacketId = nextPacketId >= 127 ? 0 : nextPacketId + 1;
  return id;
}

// ─────────────────────────────────────────────────────────────────────────────
// Response decoder  (device → host)
// We parse the device output frames off an rx ring buffer.
// Frame types:
//   0x03 <id>               ACK
//   0x04 <lastId> <reason>  NACK
//   0x08 <byte>             single-byte value
//   0x06 <len> <…> 0x20    string frame
//   0x07 <len> <…> 0x20    debug frame
//   0x09 <type> <len> <…>  custom/device-initiated packet
// ─────────────────────────────────────────────────────────────────────────────
let rxBuf = Buffer.alloc(0);

// Pending promise resolver set by waitForResponse()
let pendingResolve = null;
let pendingReject  = null;
let pendingTimer   = null;
const pendingFrames = [];   // decoded frames queued while no one is waiting

const NACK_REASONS = {
  0x01: 'bad packet ID',
  0x02: 'bad length (0 or >32)',
  0x03: 'missing CRC',
  0x04: 'CRC mismatch',
  0x05: 'missing data',
};

function tryDecodeFrames() {
  const decoded = [];

  while (rxBuf.length >= 2) {
    const type = rxBuf[0];

    if (type === 0x03) {                        // ACK
      if (rxBuf.length < 2) break;
      decoded.push({ type: 'ACK', id: rxBuf[1] });
      rxBuf = rxBuf.slice(2);
      continue;
    }

    if (type === 0x04) {                        // NACK
      if (rxBuf.length < 3) break;
      decoded.push({ type: 'NACK', lastId: rxBuf[1], reason: rxBuf[2] });
      rxBuf = rxBuf.slice(3);
      continue;
    }

    if (type === 0x08) {                        // single byte
      if (rxBuf.length < 2) break;
      decoded.push({ type: 'BYTE', value: rxBuf[1] });
      rxBuf = rxBuf.slice(2);
      continue;
    }

    if (type === 0x06 || type === 0x07) {       // string or debug frame
      if (rxBuf.length < 2) break;
      const len = rxBuf[1];
      if (rxBuf.length < 2 + len + 1) break;   // wait for terminator 0x20
      const text = rxBuf.slice(2, 2 + len).toString('latin1');
      const terminator = rxBuf[2 + len];
      if (terminator !== 0x20) {
        log('WARN', 'RX-DECODE', `Expected 0x20 terminator, got 0x${terminator.toString(16)} – skipping byte`);
        rxBuf = rxBuf.slice(1);
        continue;
      }
      decoded.push({ type: type === 0x06 ? 'STRING' : 'DEBUG', text });
      rxBuf = rxBuf.slice(3 + len);
      continue;
    }

    if (type === 0x09) {                        // custom/device-initiated packet
      if (rxBuf.length < 3) break;
      const pktType = rxBuf[1];
      const len     = rxBuf[2];
      if (rxBuf.length < 3 + len) break;
      const data = rxBuf.slice(3, 3 + len);
      decoded.push({ type: 'CUSTOM', pktType, data });
      rxBuf = rxBuf.slice(3 + len);
      continue;
    }

    // Unknown byte – discard and warn
    log('WARN', 'RX-DECODE', `Unknown frame type 0x${type.toString(16)} – discarding byte`);
    rxBuf = rxBuf.slice(1);
  }

  return decoded;
}

function dispatchDecodedFrames(frames) {
  for (const frame of frames) {
    logFrame(frame);
    if (pendingResolve) {
      clearTimeout(pendingTimer);
      const resolve = pendingResolve;
      pendingResolve = null;
      pendingReject  = null;
      pendingTimer   = null;
      resolve(frame);
    } else {
      pendingFrames.push(frame);
    }
  }
}

function logFrame(frame) {
  switch (frame.type) {
    case 'ACK':
      log('DEBUG', 'RX', `ACK  id=0x${frame.id.toString(16).padStart(2,'0')}`);
      break;
    case 'NACK':
      log('WARN',  'RX', `NACK lastId=0x${frame.lastId.toString(16).padStart(2,'0')} reason=0x${frame.reason.toString(16)} (${NACK_REASONS[frame.reason] || 'unknown'})`);
      break;
    case 'BYTE':
      log('DEBUG', 'RX', `BYTE 0x${frame.value.toString(16).padStart(2,'0')} ('${frame.value >= 0x20 && frame.value < 0x7F ? String.fromCharCode(frame.value) : '.'}')`);
      break;
    case 'STRING':
      log('DEBUG', 'RX', `STR  "${frame.text.replace(/\n/g,'\\n').replace(/\r/g,'\\r')}"`);
      break;
    case 'DEBUG':
      log('DEBUG', 'RX', `DBG  "${frame.text.replace(/\n/g,'\\n').replace(/\r/g,'\\r')}"`);
      break;
    case 'CUSTOM': {
      const typeNames = { 1: 'EncoderMove', 2: 'EncoderBtn', 3: 'Button', 4: 'TM1638Btn' };
      log('INFO', 'RX', `CUSTOM type=0x${frame.pktType.toString(16)} (${typeNames[frame.pktType] || '?'}) data=[${hexStr(frame.data)}]`);
      break;
    }
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// Promise-based receive helpers
// ─────────────────────────────────────────────────────────────────────────────

/**
 * Wait for the next decoded frame, or reject after `timeoutMs`.
 */
function nextFrame(timeoutMs = 2000) {
  return new Promise((resolve, reject) => {
    if (pendingFrames.length > 0) {
      resolve(pendingFrames.shift());
      return;
    }
    pendingResolve = resolve;
    pendingReject  = reject;
    pendingTimer = setTimeout(() => {
      pendingResolve = null;
      pendingReject  = null;
      reject(new Error(`Timeout waiting for frame (${timeoutMs} ms)`));
    }, timeoutMs);
  });
}

/**
 * Wait for an ACK for the given packetId.
 * Skips any non-ACK frames received first (they are logged separately).
 */
async function waitForAck(packetId, timeoutMs = 2000) {
  const deadline = Date.now() + timeoutMs;
  while (Date.now() < deadline) {
    const frame = await nextFrame(deadline - Date.now());
    if (frame.type === 'NACK') {
      throw new Error(`NACK received: lastId=${frame.lastId} reason=${NACK_REASONS[frame.reason] || frame.reason}`);
    }
    if (frame.type === 'ACK') {
      if (frame.id !== packetId && packetId !== BROADCAST_ID) {
        log('WARN', 'HOST', `ACK id mismatch: expected 0x${packetId.toString(16)}, got 0x${frame.id.toString(16)}`);
      }
      return frame;
    }
    // Non-ACK frame arrived before ACK – already logged, keep waiting
  }
  throw new Error('Timeout waiting for ACK');
}

/**
 * Collect response frames after an ACK until a terminator condition is met.
 *
 * @param {Function} isDone  called with (collectedFrames) → bool
 * @param {number}   timeoutMs  per-frame timeout
 */
async function collectFrames(isDone, timeoutMs = 2000) {
  const frames = [];
  while (!isDone(frames)) {
    const f = await nextFrame(timeoutMs);
    if (f.type === 'BYTE' || f.type === 'STRING' || f.type === 'DEBUG' || f.type === 'CUSTOM') {
      frames.push(f);
    }
    // ACK/NACK in the middle is unexpected but just log and continue
  }
  return frames;
}

// ─────────────────────────────────────────────────────────────────────────────
// Send helpers
// ─────────────────────────────────────────────────────────────────────────────
let port;

function sendRaw(buf, label) {
  logHex('TX', label, buf);
  port.write(buf);
}

/**
 * Send one ARQ packet and wait for ACK.
 * Returns the ACK frame.
 */
async function sendPacket(packetId, payload, label) {
  const pkt = buildArqPacket(packetId, payload);
  sendRaw(pkt, `ARQ[id=0x${packetId.toString(16).padStart(2,'0')}] ${label}`);
  const ack = await waitForAck(packetId);
  log('DEBUG', 'HOST', `ACK received for [id=0x${packetId.toString(16).padStart(2,'0')}] ${label}`);
  return ack;
}

// ─────────────────────────────────────────────────────────────────────────────
// Protocol phases
// ─────────────────────────────────────────────────────────────────────────────

async function phase1_hello() {
  log('INFO', 'PHASE-1', '── Hello ───────────────────────────────────────────');

  // Hello packet: broadcast (0xFF), payload = 0x03 '1' 0x10
  const payload = buildPayload('1', Buffer.from([0x10]));
  await sendPacket(BROADCAST_ID, payload, 'Hello');

  // Expect a BYTE frame with the firmware version
  const frame = await nextFrame(2000);
  if (frame.type !== 'BYTE') {
    throw new Error(`Expected BYTE (version), got ${frame.type}`);
  }
  const version = frame.value;
  log('INFO', 'PHASE-1',
    `Device firmware version: 0x${version.toString(16).padStart(2,'0')} ('${String.fromCharCode(version)}')`);

  if (version !== 0x6A) {
    log('WARN', 'PHASE-1', `Expected 0x6A ('j'), got 0x${version.toString(16)} – continuing anyway`);
  } else {
    log('INFO', 'PHASE-1', 'Version matches expected 0x6A ✓');
  }

  return version;
}

async function phase2_enumerate() {
  log('INFO', 'PHASE-2', '── Feature Enumeration ─────────────────────────────');

  const result = {};

  // ── Features ('0') ────────────────────────────────────────────────────────
  {
    log('INFO', 'PHASE-2', 'Querying features…');
    const id = allocPacketId();
    await sendPacket(id, buildPayload('0'), 'Features');

    // Collect STRING frames until we get one containing '\n' (terminator)
    const frames = await collectFrames(
      fs => fs.some(f => f.type === 'STRING' && f.text.includes('\n'))
    );
    const features = frames
      .filter(f => f.type === 'STRING')
      .map(f => f.text.replace(/\n/g, '').trim())
      .filter(Boolean);
    result.features = features;
    log('INFO', 'PHASE-2', `Features: [${features.join(', ')}]`);
  }

  // ── RGB LED count ('4') ───────────────────────────────────────────────────
  {
    log('INFO', 'PHASE-2', 'Querying RGB LED count…');
    const id = allocPacketId();
    await sendPacket(id, buildPayload('4'), 'RGBCount');
    const f = await nextFrame();
    if (f.type === 'BYTE') {
      result.rgbCount = f.value;
      log('INFO', 'PHASE-2', `RGB LED count: ${f.value}`);
    }
  }

  // ── TM1638 count ('2') ────────────────────────────────────────────────────
  {
    log('INFO', 'PHASE-2', 'Querying TM1638 count…');
    const id = allocPacketId();
    await sendPacket(id, buildPayload('2'), 'TM1638Count');
    const f = await nextFrame();
    if (f.type === 'BYTE') {
      result.tm1638Count = f.value;
      log('INFO', 'PHASE-2', `TM1638 count: ${f.value}`);
    }
  }

  // ── Simple modules count ('B') ────────────────────────────────────────────
  {
    log('INFO', 'PHASE-2', 'Querying simple modules count…');
    const id = allocPacketId();
    await sendPacket(id, buildPayload('B'), 'SimpleModulesCount');
    const f = await nextFrame();
    if (f.type === 'BYTE') {
      result.simpleModules = f.value;
      log('INFO', 'PHASE-2', `Simple modules count: ${f.value}`);
    }
  }

  // ── Extended command list ('X list') ─────────────────────────────────────
  {
    log('INFO', 'PHASE-2', 'Querying extended command list…');
    const id = allocPacketId();
    // Payload: 0x03 'X' 'l' 'i' 's' 't' 0x0A
    await sendPacket(id, buildPayload('X', Buffer.from('list\n')), 'X list');

    // Collect STRING frames until we see an empty-ish line or a bare BYTE 0x0A
    const frames = await collectFrames(fs => {
      const last = fs[fs.length - 1];
      if (!last) return false;
      // Terminator: firmware sends PrintLn() which is a single-byte output 0x0A
      if (last.type === 'BYTE' && last.value === 0x0A) return true;
      // Or a STRING frame whose text is just '\n'
      if (last.type === 'STRING' && last.text.trim() === '') return true;
      return false;
    }, 1500);

    const cmds = frames
      .filter(f => f.type === 'STRING')
      .map(f => f.text.replace(/\n/g, '').trim())
      .filter(Boolean);
    result.extendedCommands = cmds;
    log('INFO', 'PHASE-2', `Extended commands: [${cmds.join(', ')}]`);
  }

  // ── Device name ('N') ─────────────────────────────────────────────────────
  {
    log('INFO', 'PHASE-2', 'Querying device name…');
    const id = allocPacketId();
    await sendPacket(id, buildPayload('N'), 'DeviceName');
    const f = await nextFrame();
    if (f.type === 'STRING') {
      result.deviceName = f.text.replace(/\n/g, '').trim();
      log('INFO', 'PHASE-2', `Device name: "${result.deviceName}"`);
    }
  }

  // ── Unique ID ('I') ───────────────────────────────────────────────────────
  {
    log('INFO', 'PHASE-2', 'Querying unique ID…');
    const id = allocPacketId();
    await sendPacket(id, buildPayload('I'), 'UniqueId');
    const f = await nextFrame();
    if (f.type === 'STRING') {
      result.uniqueId = f.text.replace(/\n/g, '').trim();
      log('INFO', 'PHASE-2', `Unique ID: "${result.uniqueId}"`);
    }
  }

  // ── Button count ('J') ────────────────────────────────────────────────────
  {
    log('INFO', 'PHASE-2', 'Querying button count…');
    const id = allocPacketId();
    await sendPacket(id, buildPayload('J'), 'ButtonCount');
    const f = await nextFrame();
    if (f.type === 'BYTE') {
      result.buttonCount = f.value;
      log('INFO', 'PHASE-2', `Button count: ${f.value}`);
    }
  }

  // ── MCU type ('X mcutype') ────────────────────────────────────────────────
  {
    log('INFO', 'PHASE-2', 'Querying MCU type…');
    const id = allocPacketId();
    await sendPacket(id, buildPayload('X', Buffer.from('mcutype\n')), 'X mcutype');

    // Expects exactly 3 BYTE frames (signature bytes)
    const sig = [];
    for (let i = 0; i < 3; i++) {
      const f = await nextFrame(1500);
      if (f.type === 'BYTE') sig.push(f.value);
    }
    result.mcuSignature = sig;
    log('INFO', 'PHASE-2',
      `MCU signature: [${sig.map(b => '0x' + b.toString(16).padStart(2,'0')).join(', ')}]`);
    if (sig[0] === 0x1E && sig[1] === 0x98 && sig[2] === 0x01) {
      log('INFO', 'PHASE-2', 'Signature matches ATmega2560 ✓');
    } else {
      log('WARN', 'PHASE-2', 'Signature does not match expected ATmega2560 [0x1E, 0x98, 0x01]');
    }
  }

  return result;
}

async function phase3_keepalive(enumResult) {
  log('INFO', 'PHASE-3', '── Streaming / Keepalive ───────────────────────────');
  log('INFO', 'PHASE-3', 'Sending initial gear command then repeating keepalives. Press Ctrl-C to stop.');

  // First packet: gear display  ('G' + gear char)
  {
    const id = allocPacketId();
    await sendPacket(id, buildPayload('G', Buffer.from(['-'.charCodeAt(0)])), 'Gear=-');
    log('INFO', 'PHASE-3', 'Gear set to "-"');
  }

  let kaCount = 0;
  return new Promise(() => {   // never resolves – runs until Ctrl-C
    const interval = setInterval(async () => {
      kaCount++;
      try {
        const id = allocPacketId();
        // 'X keepalive\n'
        const pkt = buildArqPacket(id, buildPayload('X', Buffer.from('keepalive\n')));
        logHex('TX', `ARQ[id=0x${id.toString(16).padStart(2,'0')}] keepalive #${kaCount}`, pkt);
        port.write(pkt);
        // The device does not respond to keepalive; just wait briefly for any
        // device-initiated frames (buttons, encoders) before the next cycle.
        await new Promise(r => setTimeout(r, 50));
      } catch (err) {
        log('WARN', 'PHASE-3', `Keepalive error: ${err.message}`);
      }
    }, 500);

    // Drain any unexpected device-initiated frames (buttons, encoders, debug)
    // These arrive asynchronously via the normal onData path and are already
    // logged by dispatchDecodedFrames → logFrame.
  });
}

// ─────────────────────────────────────────────────────────────────────────────
// Main
// ─────────────────────────────────────────────────────────────────────────────

async function main() {
  log('INFO', 'MAIN', '═══════════════════════════════════════════════════════');
  log('INFO', 'MAIN', ' ESP-SimHub Host Simulator  (tests a real device)');
  log('INFO', 'MAIN', '═══════════════════════════════════════════════════════');
  log('INFO', 'MAIN', `Port:      ${portPath}`);
  log('INFO', 'MAIN', `Baud rate: ${baudRate}`);
  log('INFO', 'MAIN', '───────────────────────────────────────────────────────');

  await new Promise((resolve, reject) => {
    port = new SerialPort({ path: portPath, baudRate }, err => {
      if (err) reject(err); else resolve();
    });
  });

  log('INFO', 'MAIN', `Port ${portPath} opened at ${baudRate} baud`);

  port.on('data', rawData => {
    logHex('RX-RAW', 'serial', rawData);
    rxBuf = Buffer.concat([rxBuf, rawData]);
    dispatchDecodedFrames(tryDecodeFrames());
  });

  port.on('error', err => log('ERROR', 'SERIAL', err.message));
  port.on('close', () => { log('INFO', 'SERIAL', 'Port closed'); process.exit(0); });

  // Small delay to let the device settle after port open
  await new Promise(r => setTimeout(r, 500));

  try {
    await phase1_hello();
    const enumResult = await phase2_enumerate();

    log('INFO', 'SUMMARY', '───────────────────────────────────────────────────');
    log('INFO', 'SUMMARY', 'Enumeration complete:');
    for (const [k, v] of Object.entries(enumResult)) {
      log('INFO', 'SUMMARY', `  ${k.padEnd(18)} = ${Array.isArray(v) ? JSON.stringify(v) : v}`);
    }
    log('INFO', 'SUMMARY', '───────────────────────────────────────────────────');

    await phase3_keepalive(enumResult);
  } catch (err) {
    log('ERROR', 'MAIN', `Protocol error: ${err.message}`);
    log('ERROR', 'MAIN', 'Stack: ' + err.stack);
    port.close();
    process.exit(1);
  }
}

process.on('SIGINT', () => {
  log('INFO', 'MAIN', 'Interrupted – closing port');
  if (port && port.isOpen) port.close();
  process.exit(0);
});

main().catch(err => {
  log('ERROR', 'MAIN', err.message);
  process.exit(1);
});
