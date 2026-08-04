/*
 * BH1750 / GY-302 combined I2C example
 * for Seeed XIAO ESP32-S3.
 *
 * GY-302 is a common breakout board for the BH1750 light sensor, so both
 * names use the same I2C commands and measurement calculation.
 *
 * XIAO ESP32-S3 wiring:
 *   D4 / GPIO5 -> SDA
 *   D5 / GPIO6 -> SCL
 *   3V3        -> VCC
 *   GND        -> GND
 *
 * This user's BH1750/GY-302 module is configured at I2C address 0x23.
 * ADDR is therefore connected to GND.
 *
 * Use the pin labels printed on the GY-302 board because the physical pin
 * order can vary. No external library is required.
 */

#include <Arduino.h>
#include <Wire.h>

constexpr uint8_t SDA_PIN = 5;  // XIAO D4
constexpr uint8_t SCL_PIN = 6;  // XIAO D5
constexpr uint8_t BH1750_ADDRESS = 0x23;

// Change to true when you also want to list every device on the I2C bus.
// Keeping it false makes the BH1750 result appear immediately.
constexpr bool RUN_FULL_I2C_SCAN = false;

uint8_t sensorAddress = 0;

bool devicePresent(uint8_t address) {
  Wire.beginTransmission(address);
  return Wire.endTransmission() == 0;
}

bool sendCommand(uint8_t command) {
  if (sensorAddress == 0) return false;

  Wire.beginTransmission(sensorAddress);
  Wire.write(command);
  return Wire.endTransmission() == 0;
}

void scanAllI2CDevices() {
  Serial.println("Scanning all I2C addresses...");
  uint8_t found = 0;

  for (uint8_t address = 0x03; address <= 0x77; address++) {
    if (devicePresent(address)) {
      Serial.printf("  I2C device: 0x%02X", address);
      if (address == BH1750_ADDRESS) {
        Serial.print(" (BH1750/GY-302 at configured address)");
      }
      Serial.println();
      found++;
    }
  }

  if (found == 0) {
    Serial.println("  no I2C device responded");
  } else {
    Serial.printf("I2C devices found: %u\n", found);
  }
}

bool beginBH1750() {
  if (!devicePresent(BH1750_ADDRESS)) {
    sensorAddress = 0;
    return false;
  }
  sensorAddress = BH1750_ADDRESS;

  // Power on, reset, then continuous high-resolution measurement mode.
  if (!sendCommand(0x01)) return false;
  delay(10);
  if (!sendCommand(0x07)) return false;
  delay(10);
  if (!sendCommand(0x10)) return false;
  delay(180);
  return true;
}

bool readLux(float &lux, uint16_t &raw) {
  if (sensorAddress == 0) return false;

  const uint8_t received = Wire.requestFrom(sensorAddress, (uint8_t)2);
  if (received != 2 || Wire.available() < 2) return false;

  raw = ((uint16_t)Wire.read() << 8) | Wire.read();
  // High-resolution BH1750 mode reports raw / 1.2 lux.
  lux = raw / 1.2f;
  return true;
}

void setup() {
  Serial.begin(115200);
  delay(3000);  // USB CDC startup time

  Serial.println();
  Serial.println("=== BH1750 / GY-302 test ===");
  Serial.printf("I2C pins: SDA GPIO%d, SCL GPIO%d\n", SDA_PIN, SCL_PIN);

  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(100000);
  Wire.setTimeOut(50);

  if (beginBH1750()) {
    Serial.printf("BH1750/GY-302 detected at 0x%02X\n", sensorAddress);
    Serial.println("Measurement started. Reading every 1 second...");
  } else {
    Serial.println("BH1750/GY-302 not found at configured address 0x23.");
    Serial.println("Check VCC, GND, SDA, SCL, and ADDR wiring.");
  }

  if (RUN_FULL_I2C_SCAN) {
    scanAllI2CDevices();
  }
}

void loop() {
  static uint32_t lastRead = 0;
  if (millis() - lastRead < 1000) return;
  lastRead = millis();

  float lux = 0.0f;
  uint16_t raw = 0;
  if (readLux(lux, raw)) {
    Serial.printf("BH1750/GY-302 [0x%02X]: raw=%u, %.2f lux\n",
                  sensorAddress, raw, lux);
  } else if (sensorAddress != 0) {
    Serial.println("BH1750/GY-302 read failed");
  }
}
