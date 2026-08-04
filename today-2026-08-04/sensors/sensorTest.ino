/*
 * XIAO ESP32-S3 sensor test hub
 *
 * Current sensor:
 *   - BH1750 ambient light sensor
 *   - I2C bus scanner for the other sensors connected to the same bus
 *
 * XIAO ESP32-S3 wiring:
 *   D4 / GPIO5 -> SDA
 *   D5 / GPIO6 -> SCL
 *   3V3        -> VCC
 *   GND        -> GND
 *
 * BH1750 address:
 *   ADDR low/GND -> 0x23
 *   ADDR high/3V3 -> 0x5C
 *
 * This sketch uses only Wire.h, so no BH1750 library is required.
 */

#include <Arduino.h>
#include <Wire.h>

constexpr uint8_t I2C_SDA_PIN = 5;  // XIAO D4
constexpr uint8_t I2C_SCL_PIN = 6;  // XIAO D5
constexpr uint8_t BH1750_ADDR_LOW = 0x23;
constexpr uint8_t BH1750_ADDR_HIGH = 0x5C;
constexpr uint8_t BNO055_ADDR_LOW = 0x28;
constexpr uint8_t BNO055_ADDR_HIGH = 0x29;
constexpr uint8_t BME280_ADDRESS = 0x76;

uint8_t bh1750Address = 0;

bool probeI2C(uint8_t address) {
  Wire.beginTransmission(address);
  return Wire.endTransmission() == 0;
}

bool sendBH1750Command(uint8_t command) {
  if (bh1750Address == 0) return false;

  Wire.beginTransmission(bh1750Address);
  Wire.write(command);
  return Wire.endTransmission() == 0;
}

void scanI2CBus() {
  Serial.println();
  Serial.println("I2C scan start");

  uint8_t found = 0;
  for (uint8_t address = 0x03; address <= 0x77; address++) {
    if (probeI2C(address)) {
      Serial.printf("  device found: 0x%02X", address);
      if (address == BH1750_ADDR_LOW || address == BH1750_ADDR_HIGH) {
        Serial.print(" (BH1750 candidate)");
      } else if (address == BNO055_ADDR_LOW || address == BNO055_ADDR_HIGH) {
        Serial.print(" (BNO055 candidate)");
      } else if (address == BME280_ADDRESS) {
        Serial.print(" (BME280 candidate)");
      }
      Serial.println();
      found++;
    }
  }

  if (found == 0) {
    Serial.println("  no I2C device found");
  } else {
    Serial.printf("I2C devices found: %u\n", found);
  }
}

bool beginBH1750() {
  if (probeI2C(BH1750_ADDR_LOW)) {
    bh1750Address = BH1750_ADDR_LOW;
  } else if (probeI2C(BH1750_ADDR_HIGH)) {
    bh1750Address = BH1750_ADDR_HIGH;
  } else {
    bh1750Address = 0;
    return false;
  }

  // Power on, reset, then start continuous high-resolution measurement.
  if (!sendBH1750Command(0x01)) return false;
  delay(10);
  if (!sendBH1750Command(0x07)) return false;
  delay(10);
  if (!sendBH1750Command(0x10)) return false;
  delay(180);
  return true;
}

bool readBH1750(float &lux) {
  if (bh1750Address == 0) return false;

  const uint8_t received = Wire.requestFrom(bh1750Address, (uint8_t)2);
  if (received != 2 || Wire.available() < 2) return false;

  const uint16_t raw = ((uint16_t)Wire.read() << 8) | Wire.read();
  // BH1750 high-resolution mode reports raw / 1.2 lux.
  lux = raw / 1.2f;
  return true;
}

void setup() {
  Serial.begin(115200);
  delay(3000);  // allow USB CDC to enumerate

  Serial.println();
  Serial.println("=== XIAO ESP32-S3 sensorTest ===");
  Serial.printf("I2C pins: SDA GPIO%d, SCL GPIO%d\n", I2C_SDA_PIN, I2C_SCL_PIN);

  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  Wire.setClock(100000);
  Wire.setTimeOut(50);

  scanI2CBus();

  if (beginBH1750()) {
    Serial.printf("BH1750 ready at address 0x%02X\n", bh1750Address);
  } else {
    Serial.println("BH1750 not found at 0x23 or 0x5C");
    Serial.println("Check 3V3, GND, SDA(D4/GPIO5), SCL(D5/GPIO6), and ADDR.");
  }
}

void loop() {
  static uint32_t lastRead = 0;
  if (millis() - lastRead < 1000) return;
  lastRead = millis();

  float lux = 0.0f;
  if (readBH1750(lux)) {
    Serial.printf("BH1750 [0x%02X] light: %.2f lux\n", bh1750Address, lux);
  } else if (bh1750Address != 0) {
    Serial.println("BH1750 read failed");
  }
}
