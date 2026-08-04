/*
 * BME280 4-pin I2C test example for Seeed XIAO ESP32-S3.
 *
 * Target address: 0x76
 * The sketch checks CHIP_ID=0x60, reads factory calibration data, and applies
 * the Bosch compensation formulas for temperature, pressure, and humidity.
 * No external BME280 library is required.
 *
 * XIAO ESP32-S3 wiring:
 *   BME280 VCC -> 3V3
 *   BME280 GND -> GND
 *   BME280 SDA -> D4 / GPIO5
 *   BME280 SCL -> D5 / GPIO6
 *
 * Follow the pin labels on the 4-pin module. Some modules use SDO/ADR to
 * select the address: SDO low is normally 0x76 and SDO high is normally 0x77.
 */

#include <Arduino.h>
#include <Wire.h>

constexpr uint8_t SDA_PIN = 5;  // XIAO D4
constexpr uint8_t SCL_PIN = 6;  // XIAO D5
constexpr uint8_t BME280_ADDRESS = 0x76;

constexpr uint8_t REG_CHIP_ID = 0xD0;
constexpr uint8_t REG_RESET = 0xE0;
constexpr uint8_t REG_CTRL_HUM = 0xF2;
constexpr uint8_t REG_CONFIG = 0xF5;
constexpr uint8_t REG_CTRL_MEAS = 0xF4;
constexpr uint8_t REG_DATA = 0xF7;

struct CalibrationData {
  uint16_t digT1;
  int16_t digT2;
  int16_t digT3;
  uint16_t digP1;
  int16_t digP2;
  int16_t digP3;
  int16_t digP4;
  int16_t digP5;
  int16_t digP6;
  int16_t digP7;
  int16_t digP8;
  int16_t digP9;
  uint8_t digH1;
  int16_t digH2;
  uint8_t digH3;
  int16_t digH4;
  int16_t digH5;
  int8_t digH6;
};

CalibrationData calibration = {};
int32_t tFine = 0;
bool sensorReady = false;

bool probeSensor() {
  Wire.beginTransmission(BME280_ADDRESS);
  return Wire.endTransmission() == 0;
}

bool writeRegister(uint8_t reg, uint8_t value) {
  Wire.beginTransmission(BME280_ADDRESS);
  Wire.write(reg);
  Wire.write(value);
  return Wire.endTransmission() == 0;
}

bool readRegisters(uint8_t reg, uint8_t *buffer, uint8_t length) {
  Wire.beginTransmission(BME280_ADDRESS);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) return false;

  const uint8_t received = Wire.requestFrom(BME280_ADDRESS, length);
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

uint16_t readU16LE(const uint8_t *data) {
  return (uint16_t)data[0] | ((uint16_t)data[1] << 8);
}

int16_t readS16LE(const uint8_t *data) {
  return (int16_t)readU16LE(data);
}

bool readCalibration() {
  uint8_t first[26] = {};
  uint8_t humidity[7] = {};

  // 0x88..0xA1 contains temperature, pressure, and H1 calibration values.
  if (!readRegisters(0x88, first, sizeof(first))) return false;
  // 0xE1..0xE7 contains the remaining humidity calibration values.
  if (!readRegisters(0xE1, humidity, sizeof(humidity))) return false;

  calibration.digT1 = readU16LE(&first[0]);
  calibration.digT2 = readS16LE(&first[2]);
  calibration.digT3 = readS16LE(&first[4]);
  calibration.digP1 = readU16LE(&first[6]);
  calibration.digP2 = readS16LE(&first[8]);
  calibration.digP3 = readS16LE(&first[10]);
  calibration.digP4 = readS16LE(&first[12]);
  calibration.digP5 = readS16LE(&first[14]);
  calibration.digP6 = readS16LE(&first[16]);
  calibration.digP7 = readS16LE(&first[18]);
  calibration.digP8 = readS16LE(&first[20]);
  calibration.digP9 = readS16LE(&first[22]);
  calibration.digH1 = first[25];

  calibration.digH2 = readS16LE(&humidity[0]);
  calibration.digH3 = humidity[2];
  calibration.digH4 = (int16_t)(((int16_t)humidity[3] << 4) |
                                (humidity[4] & 0x0F));
  calibration.digH5 = (int16_t)(((int16_t)humidity[5] << 4) |
                                (humidity[4] >> 4));
  calibration.digH6 = (int8_t)humidity[6];
  return true;
}

float compensateTemperature(int32_t adcT) {
  int32_t var1 = ((((adcT >> 3) - ((int32_t)calibration.digT1 << 1)) *
                   (int32_t)calibration.digT2) >> 11);
  int32_t var2 = (((((adcT >> 4) - (int32_t)calibration.digT1) *
                    ((adcT >> 4) - (int32_t)calibration.digT1)) >> 12) *
                  (int32_t)calibration.digT3) >> 14;
  tFine = var1 + var2;
  // The datasheet result is in 0.01 deg C; convert it to deg C.
  return (tFine * 5 + 128) / 25600.0f;
}

float compensatePressure(int32_t adcP) {
  int64_t var1 = (int64_t)tFine - 128000;
  int64_t var2 = var1 * var1 * (int64_t)calibration.digP6;
  var2 += (var1 * (int64_t)calibration.digP5) << 17;
  var2 += ((int64_t)calibration.digP4) << 35;
  var1 = ((var1 * var1 * (int64_t)calibration.digP3) >> 8) +
         ((var1 * (int64_t)calibration.digP2) << 12);
  var1 = ((((int64_t)1 << 47) + var1) * (int64_t)calibration.digP1) >> 33;

  if (var1 == 0) return -1.0f;

  int64_t pressure = 1048576 - adcP;
  pressure = (((pressure << 31) - var2) * 3125) / var1;
  var1 = ((int64_t)calibration.digP9 * (pressure >> 13) *
          (pressure >> 13)) >> 25;
  var2 = ((int64_t)calibration.digP8 * pressure) >> 19;
  pressure = ((pressure + var1 + var2) >> 8) +
             ((int64_t)calibration.digP7 << 4);

  // The compensated result is in Pa with a Q24.8 fixed-point fraction.
  return pressure / 256.0f;
}

float compensateHumidity(int32_t adcH) {
  int32_t value = tFine - 76800;
  value = (((((adcH << 14) - ((int32_t)calibration.digH4 << 20) -
             ((int32_t)calibration.digH5 * value)) +
            16384) >> 15) *
           (((((((value * (int32_t)calibration.digH6) >> 10) *
                (((value * (int32_t)calibration.digH3) >> 11) + 32768)) >>
               10) +
             2097152) *
                (int32_t)calibration.digH2 +
            8192) >>
           14));
  value -= (((((value >> 15) * (value >> 15)) >> 7) *
             (int32_t)calibration.digH1) >>
            4);

  if (value < 0) value = 0;
  if (value > 419430400) value = 419430400;
  // The compensated humidity value is Q22.10 after the final shift.
  return (value >> 12) / 1024.0f;
}

bool readMeasurement(float &temperatureC, float &pressureHpa,
                     float &humidityPercent) {
  uint8_t data[8] = {};
  if (!readRegisters(REG_DATA, data, sizeof(data))) return false;

  const int32_t adcP = ((int32_t)data[0] << 12) |
                       ((int32_t)data[1] << 4) | (data[2] >> 4);
  const int32_t adcT = ((int32_t)data[3] << 12) |
                       ((int32_t)data[4] << 4) | (data[5] >> 4);
  const int32_t adcH = ((int32_t)data[6] << 8) | data[7];

  temperatureC = compensateTemperature(adcT);
  const float pressurePa = compensatePressure(adcP);
  humidityPercent = compensateHumidity(adcH);
  pressureHpa = pressurePa / 100.0f;
  return pressurePa >= 0.0f;
}

bool beginBME280() {
  if (!probeSensor()) {
    Serial.println("BME280 did not respond at I2C address 0x76.");
    return false;
  }

  uint8_t chipId = 0;
  if (!readRegister(REG_CHIP_ID, chipId)) {
    Serial.println("BME280 CHIP_ID read failed.");
    return false;
  }
  Serial.printf("BME280 address: 0x76, CHIP_ID: 0x%02X\n", chipId);
  if (chipId != 0x60) {
    Serial.println("CHIP_ID is not 0x60; this may be a BMP280 or another device.");
    return false;
  }

  if (!writeRegister(REG_RESET, 0xB6)) return false;
  delay(10);
  if (!readCalibration()) {
    Serial.println("BME280 calibration read failed.");
    return false;
  }

  // Humidity x1; standby 62.5 ms; filter off; temperature/pressure x1;
  // normal mode for continuous measurements.
  if (!writeRegister(REG_CTRL_HUM, 0x01)) return false;
  if (!writeRegister(REG_CONFIG, 0x20)) return false;
  if (!writeRegister(REG_CTRL_MEAS, 0x27)) return false;
  delay(100);
  return true;
}

void setup() {
  Serial.begin(115200);
  delay(3000);  // USB CDC startup time

  Serial.println();
  Serial.println("=== BME280 0x76 test ===");
  Serial.printf("I2C pins: SDA GPIO%d, SCL GPIO%d\n", SDA_PIN, SCL_PIN);

  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(100000);
  Wire.setTimeOut(50);

  sensorReady = beginBME280();
  if (sensorReady) {
    Serial.println("BME280 ready. Reading every 1 second...");
  } else {
    Serial.println("Check VCC, GND, SDA, SCL, and the 0x76 address setting.");
  }
}

void loop() {
  static uint32_t lastRead = 0;
  if (millis() - lastRead < 1000) return;
  lastRead = millis();

  if (!sensorReady) return;

  float temperatureC = 0.0f;
  float pressureHpa = 0.0f;
  float humidityPercent = 0.0f;
  if (readMeasurement(temperatureC, pressureHpa, humidityPercent)) {
    Serial.printf("BME280 [0x76]: T=%.2f C, P=%.2f hPa, H=%.2f %%\n",
                  temperatureC, pressureHpa, humidityPercent);
  } else {
    Serial.println("BME280 measurement read failed");
  }
}
