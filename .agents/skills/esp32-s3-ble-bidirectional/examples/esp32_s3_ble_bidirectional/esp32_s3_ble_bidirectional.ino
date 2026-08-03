#include <Arduino.h>
#include <BLE2902.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>

// Project-local example for the esp32-s3-ble-bidirectional Skill.

// Seeed Studio XIAO ESP32-S3 defaults.
constexpr uint8_t LED_PIN = 21;
constexpr uint8_t LED_ON_LEVEL = LOW;
constexpr uint8_t LED_OFF_LEVEL = HIGH;
constexpr uint8_t TOUCH_PIN = 1;  // D0 / GPIO1 / TOUCH1

constexpr uint32_t TOUCH_NOTIFY_INTERVAL_MS = 100;  // 10 Hz

const char *DEVICE_NAME = "bkh-touch-LED";
const char *SERVICE_UUID = "4fafc201-1fb5-459e-8fcc-c5c9c331914b";
const char *TOUCH_CHARACTERISTIC_UUID =
    "beb5483e-36e1-4688-b7f5-ea07361b26a8";
const char *LED_CHARACTERISTIC_UUID =
    "9c1f0002-7f2a-4a2d-9b8e-2e6c1d5a0002";

BLECharacteristic *touchCharacteristic = nullptr;
BLECharacteristic *ledCharacteristic = nullptr;

volatile bool deviceConnected = false;
volatile bool ledCommandPending = false;
volatile bool requestedLedState = false;
bool ledIsOn = false;
uint32_t lastTouchNotification = 0;

void setLedState(bool on) {
  ledIsOn = on;
  digitalWrite(LED_PIN, on ? LED_ON_LEVEL : LED_OFF_LEVEL);

  if (ledCharacteristic == nullptr) {
    return;
  }

  uint8_t state = on ? 1 : 0;
  ledCharacteristic->setValue(&state, sizeof(state));
  if (deviceConnected) {
    ledCharacteristic->notify();
  }
}

class LedCharacteristicCallbacks : public BLECharacteristicCallbacks {
 public:
  void onWrite(BLECharacteristic *characteristic) override {
    std::string value = characteristic->getValue();
    if (value.length() != 1) {
      return;
    }

    const uint8_t command = static_cast<uint8_t>(value[0]);
    if (command == 0x01 || command == static_cast<uint8_t>('1')) {
      requestedLedState = true;
      ledCommandPending = true;
    } else if (command == 0x00 || command == static_cast<uint8_t>('0')) {
      requestedLedState = false;
      ledCommandPending = true;
    }
  }
};

class ServerCallbacks : public BLEServerCallbacks {
 public:
  void onConnect(BLEServer *server) override {
    (void)server;
    deviceConnected = true;
    Serial.println("BLE client connected");
  }

  void onDisconnect(BLEServer *server) override {
    (void)server;
    deviceConnected = false;
    Serial.println("BLE client disconnected; restarting advertising");
    BLEDevice::startAdvertising();
  }
};

void setup() {
  Serial.begin(115200);
  delay(200);

  pinMode(LED_PIN, OUTPUT);
  setLedState(false);

  BLEDevice::init(DEVICE_NAME);

  BLEServer *server = BLEDevice::createServer();
  server->setCallbacks(new ServerCallbacks());

  BLEService *service = server->createService(SERVICE_UUID);

  touchCharacteristic = service->createCharacteristic(
      TOUCH_CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
  touchCharacteristic->addDescriptor(new BLE2902());
  touchCharacteristic->setValue("0");

  ledCharacteristic = service->createCharacteristic(
      LED_CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_READ |
          BLECharacteristic::PROPERTY_WRITE |
          BLECharacteristic::PROPERTY_WRITE_NR |
          BLECharacteristic::PROPERTY_NOTIFY);
  ledCharacteristic->addDescriptor(new BLE2902());
  ledCharacteristic->setCallbacks(new LedCharacteristicCallbacks());
  uint8_t initialLedState = 0;
  ledCharacteristic->setValue(&initialLedState, sizeof(initialLedState));

  service->start();

  BLEAdvertising *advertising = BLEDevice::getAdvertising();
  advertising->addServiceUUID(SERVICE_UUID);
  advertising->setScanResponse(true);
  advertising->setMinPreferred(0x06);
  advertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();

  Serial.println("BLE server ready");
  Serial.print("Device: ");
  Serial.println(DEVICE_NAME);
  Serial.print("Touch UUID: ");
  Serial.println(TOUCH_CHARACTERISTIC_UUID);
  Serial.print("LED UUID: ");
  Serial.println(LED_CHARACTERISTIC_UUID);
}

void loop() {
  if (ledCommandPending) {
    const bool newLedState = requestedLedState;
    ledCommandPending = false;
    setLedState(newLedState);
    Serial.print("LED = ");
    Serial.println(newLedState ? 1 : 0);
  }

  const uint32_t now = millis();
  if (deviceConnected &&
      (now - lastTouchNotification >= TOUCH_NOTIFY_INTERVAL_MS)) {
    lastTouchNotification = now;

    const uint32_t touchValue = touchRead(TOUCH_PIN);
    const String payload = String(touchValue);
    touchCharacteristic->setValue(payload.c_str());
    touchCharacteristic->notify();
  }

  delay(1);
}
