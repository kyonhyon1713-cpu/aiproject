// Standalone touch-wake LED practice for Seeed Studio XIAO ESP32-S3.
//
// 1. On reset/power-up, D0/GPIO1 is calibrated and the board enters deep sleep.
// 2. Touch D0/GPIO1 to wake the board.
// 3. After a touch wake, the onboard LED turns on and stays on.
//
// ESP32-S3 touch readings rise when the pad is touched.
// Deep sleep disables USB Serial, Wi-Fi, and Bluetooth until wake-up.

#include <Arduino.h>
#include <esp_sleep.h>
#include "esp32-hal-touch.h"

constexpr uint8_t USER_LED_PIN = 21;  // XIAO ESP32-S3 onboard LED, active-low
constexpr uint8_t TOUCH_PIN = 1;     // D0 / GPIO1 / T1
constexpr uint8_t CALIBRATION_SAMPLES = 32;
constexpr uint32_t TOUCH_WAKE_MARGIN = 2000;
constexpr uint32_t LED_ON_DURATION_MS = 10000;
constexpr uint32_t TOUCH_RELEASE_MARGIN = 1000;

RTC_DATA_ATTR uint32_t touchBaseline = 0;
RTC_DATA_ATTR uint32_t wakeCount = 0;
bool wokeByTouch = false;
unsigned long ledTurnedOnAt = 0;

void setLed(bool on) {
  // The XIAO ESP32-S3 user LED is active-low.
  digitalWrite(USER_LED_PIN, on ? LOW : HIGH);
}

uint32_t calibrateTouch() {
  Serial.println("Calibrating D0/GPIO1. Do not touch the pad...");
  delay(300);

  uint64_t total = 0;
  for (uint8_t i = 0; i < CALIBRATION_SAMPLES; ++i) {
    total += touchRead(TOUCH_PIN);
    delay(10);
  }

  return static_cast<uint32_t>(total / CALIBRATION_SAMPLES);
}

void goToTouchSleep(bool waitForRelease) {
  if (waitForRelease) {
    Serial.println("Release D0 before sleeping again...");
    while (touchRead(TOUCH_PIN) > touchBaseline + TOUCH_RELEASE_MARGIN) {
      delay(50);
    }
  }

  const uint32_t wakeThreshold = touchBaseline + TOUCH_WAKE_MARGIN;

  // For ESP32-S3, touchSleepWakeUpEnable uses the absolute threshold.
  touchSleepWakeUpEnable(TOUCH_PIN, wakeThreshold);

  Serial.print("Touch wake enabled on D0/GPIO1 (T1), threshold = ");
  Serial.println(wakeThreshold);
  Serial.println("Touch D0/GPIO1 to wake and turn the LED on.");
  Serial.flush();
  delay(50);
  esp_deep_sleep_start();
}

void setup() {
  pinMode(USER_LED_PIN, OUTPUT);
  setLed(false);

  Serial.begin(115200);
  delay(200);

  const esp_sleep_wakeup_cause_t wakeupCause = esp_sleep_get_wakeup_cause();
  wokeByTouch = (wakeupCause == ESP_SLEEP_WAKEUP_TOUCHPAD);

  if (wokeByTouch) {
    ++wakeCount;
    setLed(true);
    ledTurnedOnAt = millis();

    Serial.print("Touch wake detected. LED ON. Wake count: ");
    Serial.println(wakeCount);
    Serial.print("Wake touch pad number: ");
    Serial.println(esp_sleep_get_touchpad_wakeup_status());
    return;
  }

  setLed(false);
  touchBaseline = calibrateTouch();
  Serial.print("Touch baseline = ");
  Serial.println(touchBaseline);
  goToTouchSleep(false);
}

void loop() {
  if (wokeByTouch &&
      millis() - ledTurnedOnAt >= LED_ON_DURATION_MS) {
    setLed(false);
    Serial.println("10 seconds elapsed. LED OFF.");
    goToTouchSleep(true);
  }
}
