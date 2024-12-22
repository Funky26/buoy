
unsigned long timeSaved = 0;
//sensor for detecting water in buoy(sensor for water surface)
//S  analog
//+  power(digital pin)
//-  ground
const int waterSensorRead = 34;
const int waterSensorPower = 25;
bool isWater= false;
int waterLevel;
void setup() {
  Serial.begin(115200);

  // set up of water detecting sensor
  
  pinMode(waterSensorPower, OUTPUT);

  delay(1000);
}

void loop() {


  //checking every minute
  if (millis() - timeSaved > 60000){
    //checking for water
    Serial.print("Water in buoy:");
    checkForWater();

    timeSaved = millis();
  }
}


void checkForWater()
{
  digitalWrite(waterSensorPower, HIGH);
  delay(10);
  int waterLevel = analogRead(waterSensorRead);
  Serial.println(waterLevel);
}