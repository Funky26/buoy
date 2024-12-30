#include <Adafruit_BMP085.h>
#include <Wire.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <DallasTemperature.h>
#include <OneWire.h>

unsigned long timeSaved = 0;

int timebetweenreadings= 10000;

//BLUETOOTH SETUP
BLECharacteristic *pCharacteristic;
bool deviceConnected = false;
String receivedMsg;

#define SERVICE_UUID           "<insert-UUID-here>"
#define CHARACTERISTIC_UUID_RX "<insert-UUID-here>"
#define CHARACTERISTIC_UUID_TX "<insert-UUID-here>"

//sensor for detecting water in buoy(sensor for water surface)
//S  analog
//+  power(digital pin)
//-  ground
const int waterSensorRead = 34;
const int waterSensorPower = 25;

//pressure sensor
//vcc  power
//gnd - ground
//scl (must be I2C) 22
//sda  (must be I2C) 21
Adafruit_BMP085 bmp180;
int correction = 32;
bool pressureSensor = true;

//temperature sensor DS18B20
//
//
const int temperaturepin = 4;
OneWire oneWireDS(temperaturepin);
DallasTemperature temperatureSensor(&oneWireDS);

class MyServerCallbacks: public BLEServerCallbacks {
    // při spojení zařízení nastav proměnnou na log1
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };
    // při odpojení zařízení nastav proměnnou na log0
    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};


class MyCallbacks: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    receivedMsg = pCharacteristic->getValue(); // Use the corrected global variable name
    if (receivedMsg.length() > 0) {
      Serial.print("Received message via BLE: ");
      Serial.println(receivedMsg.c_str());
    }
  }
};

//class MyCallbacks: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    receivedMsg = pCharacteristic->getValue();
    if (receivedMsg.length() > 0) {
      Serial.print("Received message via BLE: ");
      Serial.println(receivedMsg.c_str());
    }
  }
//};

void setup() {
  Serial.begin(115200);

  //*****BLE*****

  BLEDevice::init("ESP32 BLE");
  // vytvoření BLE serveru
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  // vytvoření BLE služby
  BLEService *pService = pServer->createService(SERVICE_UUID);
  // vytvoření BLE komunikačního kanálu pro odesílání (TX)
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID_TX,
                      BLECharacteristic::PROPERTY_NOTIFY
                    );
  pCharacteristic->addDescriptor(new BLE2902());
  // vytvoření BLE komunikačního kanálu pro příjem (RX)
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID_RX,
                                         BLECharacteristic::PROPERTY_WRITE
                                       );
  pCharacteristic->setCallbacks(new MyCallbacks());
  // zahájení BLE služby
  pService->start();
  // zapnutí viditelnosti BLE
  pServer->getAdvertising()->start();
  Serial.println("BLE adjusted, waiting for connecting..");
  delay(5000);

  //******SENSORS*******

  // set up of water detecting sensor
  pinMode(waterSensorPower, OUTPUT);
  //setup for pressure sensor
  bmp180.begin();
  if (!bmp180.begin()) {
    Serial.println("Could not find a valid BMP085 sensor, check wiring!");
    pressureSensor = false;
  }
  else
  {
    Serial.println("BMP sensor found.");
  }

  temperatureSensor.begin();

  delay(1000);
}

void loop() {

  //*****BLE*****
  if (deviceConnected) {
    sendBLEMessage("Device connected, sending sensor data...");
  }

  //checking every minute
  if (millis() - timeSaved > timebetweenreadings){
    //checking for water
    Serial.print("Water in buoy:");
    checkForWater();

    if (pressureSensor) {
      checkForPressure();
    } else {
      Serial.println("Pressure sensor not available.");
    }
    
    checkForTemperature();

    Serial.println("-------------------------------");
    delay(1000);
    timeSaved = millis();
  }
}

void checkForPressure()
{
  float pressure = bmp180.readPressure() + correction * 100;
  if (pressure != 0) {
    Serial.print("Pressure: ");
    Serial.print(pressure / 100.00);
    Serial.println(" hPa");

    if (deviceConnected) {
      sendBLEMessage("Pressure: " + String(pressure / 100.00) + " hPa");
    }
  } else {
    Serial.println("Pressure reading invalid or zero.");
    if (deviceConnected) {
      sendBLEMessage("Pressure reading invalid.");
    }
  }
}
void checkForTemperatureAbove() {
  float temperature = bmp180.readTemperature();
  if (temperature == 0) { // Replace 0 with another value if it’s common for invalid readings
    Serial.println("Temperature reading failed or 0.");
    sendBLEMessage("Temperature reading failed or 0.");
  } else {
    Serial.print(temperature);
    Serial.println("°C");

    // Format the temperature data into a message
    String message = "Temperature: " + String(temperature) + "°C";
    sendBLEMessage(message);
  }
}

void checkForTemperature(){
  temperatureSensor.requestTemperatures();

  float temperature = temperatureSensor.getTempCByIndex(0);

  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println("°C");

  String message = "Temperature: " + String(temperature) + "°C";
  sendBLEMessage(message);
}

void checkForWater() {
  digitalWrite(waterSensorPower, HIGH);
  delay(10);
  
  int waterLevel = analogRead(waterSensorRead);
  Serial.println(waterLevel);

  // Send water level via BLE
  String message = "Water level: " + String(waterLevel);
  sendBLEMessage(message);

  delay(1000);
  digitalWrite(waterSensorPower, LOW);
}

void sendBLEMessage(String message) {
  pCharacteristic->setValue(message.c_str());
  pCharacteristic->notify();
  delay(10); // Avoid flooding the BLE channel
}