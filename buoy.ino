#include <Adafruit_BMP085.h>
#include <Wire.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

unsigned long timeSaved = 0;

int timebetweenreadings= 10000;

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

void setup() {
  Serial.begin(115200);

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

  delay(1000);
}

void loop() {


  //checking every minute
  if (millis() - timeSaved > timebetweenreadings){
    //checking for water
    Serial.print("Water in buoy:");
    checkForWater();

    if(pressureSensor== true)
    {
      //temperature above the water
      Serial.print("Temperature (above the water): ");
      checkForTemperatureAbove();

      //atmospheric pressure
      Serial.print("Pressure: ");
      checkForPressure();
      Serial.println(" hPa");
    }
    else
    {
      Serial.println("Could not meassure pressure and temperature above water.");
    }
    

    Serial.println("-------------------------------");
    delay(1000);
    timeSaved = millis();
  }
}

void checkForPressure()
{
  float pressure = bmp180.readPressure() + correction * 100;
  if (pressure != 0) {
    Serial.print(pressure / 100.00);
  } else {
    Serial.println("Pressure reading invalid or zero 0");
  }
}
void checkForTemperatureAbove()
{
  float temperature = bmp180.readTemperature();
  if (temperature == 0) { // Replace 0 with another value if it’s common for invalid readings
    Serial.println("Temperature reading failed or 0.");
  } else {
    Serial.print(temperature);
    Serial.println("°C");
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