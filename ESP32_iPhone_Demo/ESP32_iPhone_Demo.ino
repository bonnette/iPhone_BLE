/*
    Video: https://www.youtube.com/watch?v=oCMOYS71NIU
    Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleNotify.cpp
    Ported to Arduino ESP32 by Evandro Copercini
    
    Modified 1/2019 by Larry Bonnette to add communications between an iPhone and the ESP32
    Changed service and charateristic UUID to DF01, DF02, and DF03 to make it easier to play with.
    Added notification of LED state change sent to iPhone

   Create a BLE server that, once we receive a connection, will send periodic notifications.
   The service advertises itself as: 6E400001-B5A3-F393-E0A9-E50E24DCCA9E
   Has a characteristic of: 6E400002-B5A3-F393-E0A9-E50E24DCCA9E - used for receiving data with "WRITE" 
   Has a characteristic of: 6E400003-B5A3-F393-E0A9-E50E24DCCA9E - used to send data with  "NOTIFY"

   The design of creating the BLE server is:
   1. Create a BLE Server
   2. Create a BLE Service
   3. Create a BLE Characteristic on the Service
   4. Create a BLE Descriptor on the characteristic
   5. Start the service.
   6. Start advertising.

   In this example rxValue is the data received (only accessible inside that function).
   And txValue is the data to be sent, in this example just a byte incremented every second. 
*/
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

BLECharacteristic *pCharacteristic;
bool deviceConnected = false;
bool ledState = false;
int ledChanged = 2; 
const int readPin = 32; // Use GPIO number. See ESP32 board pinouts
const int LED = 2; // Could be different depending on the dev board. I used the DOIT ESP32 dev board.

//std::string rxValue; // Could also make this a global var to access it in loop()

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

/*#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"*/
#define SERVICE_UUID           "DF01" // UART service UUID
#define CHARACTERISTIC_UUID_RX "DF02"
#define CHARACTERISTIC_UUID_TX "DF03"


class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    // On first connect send led status to iPhone
    if (ledState == false){
    pCharacteristic->setValue("X"); // Sending OFF message to iPhone
    pCharacteristic->notify(); // Send the value to the iPhone!
    }
    if (ledState == true){
    pCharacteristic->setValue("N"); // Sending On message to iPhone
    pCharacteristic->notify(); // Send the value to the iPhone!
    }
   };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();

      if (rxValue.length() > 0) {
        Serial.println("*********");
        Serial.print("Received Value: ");

        for (int i = 0; i < rxValue.length(); i++) {
          Serial.print(rxValue[i]);
        }

        Serial.println();

        // Do stuff based on the command received from the app
        if (rxValue.find(41) != -1) { 
          Serial.print("Turning ON!");
          digitalWrite(LED, HIGH);
          ledState = true;
          ledChanged = 1;
        }
        else if (rxValue.find(42) != -1) {
          Serial.print("Turning OFF!");
          digitalWrite(LED, LOW);
          ledState = false;
          ledChanged = 0;
        }

        Serial.println();
        Serial.println("*********");
      }
    }
};

void setup() {
  Serial.begin(115200);

  pinMode(LED, OUTPUT);

  // Create the BLE Device
  BLEDevice::init("ESP"); // Give it a name

  // Create the BLE Server
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID_TX,
                      BLECharacteristic::PROPERTY_NOTIFY
                    );
                      
  pCharacteristic->addDescriptor(new BLE2902());

  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID_RX,
                                         BLECharacteristic::PROPERTY_WRITE
                                       );

  pCharacteristic->setCallbacks(new MyCallbacks());

  // Start the service
  pService->start();

  // Start advertising
  pServer->getAdvertising()->start();
  Serial.println("Waiting a client connection to notify...");
}

void loop() {
  if (deviceConnected) {
    
//   pCharacteristic->setValue(&txValue, 1); // To send the integer value

// if a change to the led state is sent from the iPhone. Send the correct (on/ off) information to the phone
// Notify the phone that a change in the LED state has occured.
// Then set the ledChanged to "2" so that nothing more is sent to the phone (saves battery power)

 if (ledChanged == 1) {
    pCharacteristic->setValue("N"); // Sending ON message to iPhone
    pCharacteristic->notify(); // Send the value to the iPhone!
    ledChanged = 2;
    ledState = true;
    }
    
 if (ledChanged == 0)
    {
    pCharacteristic->setValue("X"); // Sending OFF message to iPhone
    pCharacteristic->notify(); // Send the value to the iPhone!
    ledChanged = 2;
    ledState = false;
    }
    
//    pCharacteristic->setValue(txString);
//    pCharacteristic->setValue(txValue);
    
//    pCharacteristic->notify(); // Send the value to the app!
//    Serial.print("*** Sent Value: ");
//    Serial.print(txString);
//    Serial.println(" ***");
  }
  delay(1000);
}
