#include <Adafruit_BMP085.h>
#include <Wire.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

unsigned long timeSaved = 0;


//sensor for detecting water in buoy(sensor for water surface)
//S  analog
//+  power(digital pin)
//-  ground
const int waterSensorRead = 34;
const int waterSensorPower = 25;
bool isWater= false;
int waterLevel;//not used

//pressure sensor
//vcc  power
//gnd - ground
//scl (must be I2C) 22
//sda  (must be I2C) 21
Adafruit_BMP085 bmp180;
int correction = 32;

void setup() {
  Serial.begin(115200);

  // set up of water detecting sensor
  pinMode(waterSensorPower, OUTPUT);
  //setup for pressure sensor
  bmp180.begin();

  delay(1000);
}

void loop() {


  //checking every minute
  if (millis() - timeSaved > 60000){
    //checking for water
    Serial.print("Water in buoy:");
    checkForWater();

    //temperature above the water
    float temperature = bmp180.readTemperature();
    if (temperature == 0) { // Replace 0 with another value if it’s common for invalid readings
      Serial.println("Temperature reading failed.");
    } else {
      Serial.print("Temperature (above the water): ");
      Serial.print(temperature);
      Serial.println("°C");
    }

    //atmospheric pressure
    Serial.print("Pressure: ");

    //Serial.print((bmp180.readPressure() + correction*100)/100.00);
    
    float pressure = bmp180.readPressure() + correction * 100;
    if (pressure != 0) {
      Serial.print(pressure / 100.00);
    } else {
      Serial.println("Pressure reading invalid or zero, skipping division.");
    }

    Serial.println(" hPa");

    Serial.println("-------------------------------");
    delay(1000);
    timeSaved = millis();
  }
}


void checkForWater()
{
  digitalWrite(waterSensorPower, HIGH);
  delay(10);
  int waterLevel = analogRead(waterSensorRead);
  Serial.println(waterLevel);
  delay(1000);
  digitalWrite(waterSensorPower, LOW);
}