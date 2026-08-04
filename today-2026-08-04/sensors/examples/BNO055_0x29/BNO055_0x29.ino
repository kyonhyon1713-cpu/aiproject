/*
 * BNO055 8-pin I2C test example for Seeed XIAO ESP32-S3.
 *
 * The sketch targets the user's BNO055 address 0x29 and verifies the chip ID.
 * It prints Euler angles, acceleration, gyroscope, temperature, and the
 * calibration status once per second. No external library is required.
 *
 * XIAO ESP32-S3:
 *   D4 / GPIO5 -> SDA
 *   D5 / GPIO6 -> SCL
 *   3V3        -> VCC (follow the module's VCC/VIN label)
 *   GND        -> GND
 *
 * Use the pin labels on the 8-pin module. For I2C mode, PS0/PS1 must be in
 * the module's I2C/default state. RST and INT are not required for this test.
 * The ADR/COM3 setting for address 0x29 must be HIGH on modules that expose it.
 */

#include <Arduino.h>
#include <Wire.h>

constexpr uint8_t SDA_PIN = 5;       // XIAO D4
constexpr uint8_t SCL_PIN = 6;       // XIAO D5
constexpr uint8_t BNO055_ADDR = 0x29;
constexpr uint8_t BNO055_ALT_ADDR = 0x28;
constexpr bool TRY_ALT_ADDRESS = true;

// BNO055 register addresses, page 0.
constexpr uint8_t REG_CHIP_ID = 0x00;
constexpr uint8_t REG_ACCEL_DATA = 0x08;
constexpr uint8_t REG_GYRO_DATA = 0x14;
constexpr uint8_t REG_EULER_DATA = 0x1A;
constexpr uint8_t REG_TEMP = 0x34;
constexpr uint8_t REG_CALIB_STAT = 0x35;
constexpr uint8_t REG_UNIT_SEL = 0x3B;
constexpr uint8_t REG_OPR_MODE = 0x3D;
constexpr uint8_t REG_PAGE_ID = 0x07;

uint8_t sensorAddress = BNO055_ADDR;
bool sensorReady = false;

bool probe(uint8_t address) {
  Wire.beginTransmission(address);
  return Wire.endTransmission() == 0;
}

bool writeRegister(uint8_t reg, uint8_t value) {
  Wire.beginTransmission(sensorAddress);
  Wire.write(reg);
  Wire.write(value);
  return Wire.endTransmission() == 0;
}

bool readRegisters(uint8_t reg, uint8_t *buffer, uint8_t length) {
  Wire.beginTransmission(sensorAddress);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) return false;

  const uint8_t received = Wire.requestFrom(sensorAddress, length);
  if (received != length) {
    while (Wire.available()) Wire.read();
    return false;
  }

  for (uint8_t i = 0; i < length; i++) {
    buffer[i] = Wire.read();
  }
  return true;
}

bool readRegister(uint8_t reg, uint8_t &value) {
  return readRegisters(reg, &value, 1);
}

bool readVector3(uint8_t reg, int16_t &x, int16_t &y, int16_t &z) {
  uint8_t data[6] = {};
  if (!readRegisters(reg, data, sizeof(data))) return false;

  x = (int16_t)((uint16_t)data[0] | ((uint16_t)data[1] << 8));
  y = (int16_t)((uint16_t)data[2] | ((uint16_t)data[3] << 8));
  z = (int16_t)((uint16_t)data[4] | ((uint16_t)data[5] << 8));
  return true;
}

bool beginBNO055() {
  if (!probe(sensorAddress)) {
    if (!TRY_ALT_ADDRESS || !probe(BNO055_ALT_ADDR)) {
      Serial.println("BNO055 did not respond at 0x29 (or 0x28).");
      return false;
    }
    sensorAddress = BNO055_ALT_ADDR;
    Serial.println("BNO055 found at alternate address 0x28.");
  }

  uint8_t chipId = 0;
  if (!readRegister(REG_CHIP_ID, chipId)) {
    Serial.println("BNO055 responded, but CHIP_ID read failed.");
    return false;
  }
  Serial.printf("BNO055 address: 0x%02X, CHIP_ID: 0x%02X\n", sensorAddress, chipId);
  if (chipId != 0xA0) {
    Serial.println("Warning: CHIP_ID is not 0xA0; check the module type/address.");
  }

  // Enter CONFIG mode before changing units and then start NDOF fusion mode.
  if (!writeRegister(REG_PAGE_ID, 0x00)) return false;
  if (!writeRegister(REG_OPR_MODE, 0x00)) return false;
  delay(30);
  // Default units: Celsius, m/s^2, degrees, dps.
  if (!writeRegister(REG_UNIT_SEL, 0x00)) return false;
  if (!writeRegister(REG_OPR_MODE, 0x0C)) return false;  // NDOF
  delay(700);

  return true;
}

void printMeasurement() {
  int16_t ax = 0, ay = 0, az = 0;
  int16_t gx = 0, gy = 0, gz = 0;
  int16_t heading = 0, roll = 0, pitch = 0;
  uint8_t calibration = 0;
  uint8_t temperature = 0;

  const bool accelOk = readVector3(REG_ACCEL_DATA, ax, ay, az);
  const bool gyroOk = readVector3(REG_GYRO_DATA, gx, gy, gz);
  const bool eulerOk = readVector3(REG_EULER_DATA, heading, roll, pitch);
  const bool calibOk = readRegister(REG_CALIB_STAT, calibration);
  const bool tempOk = readRegister(REG_TEMP, temperature);

  if (!accelOk || !gyroOk || !eulerOk || !calibOk || !tempOk) {
    Serial.println("BNO055 register read failed");
    return;
  }

  // BNO055 scales: accel=100 LSB/(m/s^2), gyro=16 LSB/dps,
  // Euler=16 LSB/degree.
  Serial.printf(
      "Euler H/R/P: %.2f/%.2f/%.2f deg | "
      "Accel X/Y/Z: %.2f/%.2f/%.2f m/s^2 | "
      "Gyro X/Y/Z: %.2f/%.2f/%.2f dps | "
      "Temp: %d C | Cal S/G/A/M: %u/%u/%u/%u\n",
      heading / 16.0f, roll / 16.0f, pitch / 16.0f,
      ax / 100.0f, ay / 100.0f, az / 100.0f,
      gx / 16.0f, gy / 16.0f, gz / 16.0f,
      (int8_t)temperature,
      (calibration >> 6) & 0x03, (calibration >> 4) & 0x03,
      (calibration >> 2) & 0x03, calibration & 0x03);
}

void setup() {
  Serial.begin(115200);
  delay(3000);  // USB CDC startup time

  Serial.println();
  Serial.println("=== BNO055 0x29 test ===");
  Serial.printf("I2C pins: SDA GPIO%d, SCL GPIO%d\n", SDA_PIN, SCL_PIN);

  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(100000);
  Wire.setTimeOut(50);

  sensorReady = beginBNO055();
  if (sensorReady) {
    Serial.println("BNO055 ready. Hold the board still for calibration.");
  } else {
    Serial.println("Check VCC, GND, SDA, SCL, I2C mode pins, and ADR/COM3.");
  }
}

void loop() {
  static uint32_t lastRead = 0;
  if (millis() - lastRead < 1000) return;
  lastRead = millis();

  if (sensorReady) printMeasurement();
}
