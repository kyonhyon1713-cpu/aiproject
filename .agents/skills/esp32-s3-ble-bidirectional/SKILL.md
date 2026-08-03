---
name: esp32-s3-ble-bidirectional
description: Create, adapt, compile, and explain Arduino BLE firmware for a Seeed XIAO ESP32-S3 that continuously notifies nRF Connect of touchRead values, accepts 0/1 writes for the onboard active-low LED, and resumes advertising after disconnect. Use when a user asks for ESP32-S3 BLE, nRF Connect, touch notifications, BLE writes, phone-to-board control, or bidirectional BLE communication.
---

<!-- This project-local skill is intentionally bundled with its Arduino example. -->
# ESP32-S3 BLE bidirectional communication

Use this skill to build a small, self-contained Arduino BLE GATT server for the
Seeed Studio XIAO ESP32-S3. The phone is the BLE client and nRF Connect is the
test/control app. Keep the example independent from MQTT, Wi-Fi/AP mode, and
deep sleep unless the user explicitly asks to combine them.

## Source and hardware assumptions

This skill follows the server, characteristic, `BLE2902`, and notification
patterns documented in the project's `BLE.md`.

- Board: `esp32:esp32:XIAO_ESP32S3`.
- BLE library: the classic Arduino-ESP32 Bluedroid headers supplied by the
  ESP32 board package (`BLEDevice.h`, `BLEServer.h`, `BLEUtils.h`, and
  `BLE2902.h`). Do not use `BluetoothSerial`; ESP32-S3 provides BLE, not
  Bluetooth Classic.
- Onboard LED: GPIO21, active-low. Write `LOW` to turn it on and `HIGH` to
  turn it off.
- Default touch input: D0 / GPIO1 / ESP32-S3 `TOUCH1`. Keep the pin as a
  named constant so a different XIAO touch pin can be selected safely.
- The XIAO antenna should be installed for reliable phone connections.

## GATT contract

Use one custom service with two characteristics. Keep these UUIDs stable when
the user is integrating with an existing nRF Connect setup.

| Item | UUID | Properties | Payload |
| --- | --- | --- | --- |
| Service | `4fafc201-1fb5-459e-8fcc-c5c9c331914b` | — | — |
| Touch | `beb5483e-36e1-4688-b7f5-ea07361b26a8` | Read, Notify | Decimal `touchRead()` value as UTF-8 text, normally every 100 ms |
| LED control | `9c1f0002-7f2a-4a2d-9b8e-2e6c1d5a0002` | Read, Write, Write Without Response, Notify | One byte `0x00`/`0x01`, or ASCII `0`/`1` |

The service and touch UUIDs come from `BLE.md`; the LED UUID is a second,
custom characteristic needed for the bidirectional control path. The default
advertised device name in the bundled example is `bkh-touch-LED`.

### Write behavior

Implement a `BLECharacteristicCallbacks::onWrite` callback for the LED
characteristic. Accept exactly one command byte:

- `0x01` or ASCII `'1'`: set GPIO21 `LOW` and report LED state `1`.
- `0x00` or ASCII `'0'`: set GPIO21 `HIGH` and report LED state `0`.
- Any other payload: ignore it and leave the current LED state unchanged.

Use a `BLE2902` descriptor on every Notify characteristic. nRF Connect must
enable the Client Characteristic Configuration descriptor before notifications
will arrive.

### Touch notification behavior

Read the raw ESP32-S3 touch value with `touchRead(TOUCH_PIN)` while a client is
connected. Send the value as a decimal string at the configured interval (the
example uses 100 ms / 10 Hz). This makes the value directly readable in nRF
Connect. Do not convert it to a boolean unless the user explicitly requests a
touch/no-touch protocol; ESP32-S3 touch values are useful as raw measurements
and generally rise when a finger is applied.

### Disconnect behavior

Track connection state with `BLEServerCallbacks`. In `onDisconnect`, set the
state to disconnected and call `BLEDevice::startAdvertising()` so another
phone can discover the board without a reset. Advertise the service UUID and
enable scan response.

## Standard workflow

1. Inspect the current board, port, ESP32 core, and existing sketch before
   changing anything. Preserve unrelated project files.
2. Start from
   `examples/esp32_s3_ble_bidirectional/esp32_s3_ble_bidirectional.ino` and
   keep the sketch folder name and `.ino` basename aligned for Arduino CLI.
3. Adjust only named constants such as `DEVICE_NAME`, `TOUCH_PIN`, and the
   notification interval unless the user requests a protocol change.
4. Compile with the installed ESP32-S3 board definition:

   ```powershell
   arduino-cli compile --fqbn esp32:esp32:XIAO_ESP32S3 <path-to-sketch-folder>
   ```

5. Upload only when the user explicitly requests flashing hardware. For the
   user's usual port, the standard command is:

   ```powershell
   arduino-cli upload -p COM9 --fqbn esp32:esp32:XIAO_ESP32S3 <path-to-sketch-folder>
   ```

   Confirm the resolved board and port before uploading; compilation alone must
   not alter the connected board.
6. Report the generated files, compile result, selected GPIO/UUIDs, and the
   exact nRF Connect steps.

## nRF Connect procedure

1. Open **Scanner**, grant Bluetooth/nearby-device permission, and scan for
   `bkh-touch-LED`.
2. Connect to the device and expand service
   `4fafc201-1fb5-459e-8fcc-c5c9c331914b`.
3. On the Touch characteristic
   `beb5483e-36e1-4688-b7f5-ea07361b26a8`, tap the **Notify** button (the
   crossed-out notification icon) to enable notifications. Touch D0/GPIO1 and
   watch the decimal values update.
4. On the LED control characteristic
   `9c1f0002-7f2a-4a2d-9b8e-2e6c1d5a0002`, choose **Write**. Use **HEX** mode
   and send `01` for ON or `00` for OFF. If using text mode, send the single
   character `1` or `0`.
5. If the phone disconnects, return to Scanner and connect again. The board
   should already be advertising; no reset is required.

## Verification checklist

- The sketch compiles for `esp32:esp32:XIAO_ESP32S3` with no external library
  installation beyond the ESP32 Arduino core.
- nRF Connect discovers the expected device and service UUID.
- Touch notifications change at roughly 10 Hz while connected.
- `01`/`1` lights the active-low onboard LED; `00`/`0` turns it off.
- Invalid writes do not change the LED.
- Disconnecting and rescanning finds the device again.
- Existing sketches, Wi-Fi/MQTT settings, and the user's current firmware are
  left unchanged unless explicitly included in the request.

### Confirmed hardware test

The user uploaded the bundled firmware to a real ESP32-S3 and tested it with
nRF Connect. The following runtime behaviors were confirmed on hardware:

- Touch sensor values were delivered through Notify correctly.
- Receiving `01` turned the onboard LED on.
- Receiving `00` turned the onboard LED off.
- Disconnecting the phone caused the ESP32-S3 to advertise again successfully.

Treat these behaviors as completed hardware validation when maintaining or
extending this skill; preserve them as regression requirements.

## Troubleshooting

- If the device is invisible, install the antenna, enable phone Bluetooth and
  nearby-device permission, and scan while the sketch is running.
- If no values appear, connect first and enable Notify; merely connecting does
  not subscribe the client.
- If writes have no effect, select the LED characteristic rather than the Touch
  characteristic and send one byte, not the two-character string `01`.
- If GPIO21 behaves inversely on a different board, verify that board's LED
  polarity and update `LED_ON_LEVEL`/`LED_OFF_LEVEL` in the example.
- If the user's board is not the XIAO ESP32-S3, ask for its exact board and
  touch-capable pin mapping before changing pin constants.
