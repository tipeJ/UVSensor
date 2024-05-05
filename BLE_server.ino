#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <Wire.h>
#include "Adafruit_LTR390.h"

Adafruit_LTR390 ltr = Adafruit_LTR390();

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

BLEServer *pServer;
BLEService *pService;
BLECharacteristic *pCharacteristic;
bool deviceConnected = false;

class MyServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    }

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

void setup() {
  Serial.begin(115200);
  if ( ! ltr.begin() ) {
    Serial.println("Couldn't find LTR sensor!");
    while (1) delay(10);
  }
  Serial.println("Found LTR sensor!");

  ltr.setMode(LTR390_MODE_UVS);
  if (ltr.getMode() == LTR390_MODE_ALS) {
    Serial.println("In ALS mode");
  } else {
    Serial.println("In UVS mode");
  }
  Serial.println("Starting BLE work!");

  BLEDevice::init("UV_BLE_APP");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_NOTIFY
                                       );

  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  Serial.println("Characteristic defined! Now you can read it on your phone!");
}

void loop() {
  if (deviceConnected) {
    // Read UV light data from LTR390 sensor
    float uvData = ltr.readUVS();

    // Get the current timestamp as a string
    String timestamp = String(millis() / 1000);

    // Create a string that combines timestamp, UV data, and ambient light data
    String combinedData = String(uvData);

    // Set the combined data as the value of the BLE characteristic
    pCharacteristic->setValue(combinedData.c_str());
    Serial.println(combinedData.c_str());

    // Notify the subscribed devices with the combined data
    pCharacteristic->notify();
  }

  // Delay for 1 second before the next notification
  delay(1000);
}
