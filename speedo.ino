/*
  Arduino speedometer
  @Author Yudish Appadoo
  @Email  yud.me@icloud.com
  @Repo   github.com/yudlab
 
*/
#include <SPI.h>
#include <SD.h>
#include <ArduinoJson.h>

int braudRate = 9600;
int debounceRate = 100; //in ms
int refreshRate = 500; //in ms
float wheelCirc = 1.93; // wheel circumference (in meters)

// constants won't change. Used here to set a pin number:
const int ledPin =  LED_BUILTIN;// the number of the LED pin

// Variables will change:
int ledState = LOW; // ledState used to set the LED
unsigned long previousMillis = 0; // will store last time LED was updated
const long interval = 600; // ms

uint8_t HALL_SENSOR = D2; // Pin where the hall sensor is connected to
const char* configFile = "config.odo"; //file containing the configuration settings

File SDCon;
String speedoSDData;

float distanceKM, speedp, start, speedk, lastRev, lastRefresh;
bool updateSD = true;
int elapsed;

void ICACHE_RAM_ATTR speedCalc ();

void setup ()
  {
    pinMode(LED_BUILTIN, OUTPUT);
    Serial.println();
    Serial.begin(braudRate);
    while (!Serial) {
      ; /* wait for serial port to connect. Needed for native USB port only
      *** You would want to comment this if on PROD */
    }

    if(!getConfig()) {
      distanceKM = 0;
      updateSD = false;
    }
    //toggle function speedCalc when HALL Sensor closes
    attachInterrupt(digitalPinToInterrupt(HALL_SENSOR), speedCalc, RISING); // interrupt called when sensors sends digital 2 high (every wheel rotation)

    //start now (it will be reset by the interrupt after calculating revolution time)
    start = millis(); 

    // Print a transitory message to the LCD.
    Serial.println("Hello Yudish.");
    delay(5000); //just to allow you to read the initialization message
  }

int blink ()
  {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
      // save the last time you blinked the LED
      previousMillis = currentMillis;

      // if the LED is off turn it on and vice-versa:
      if (ledState == LOW) {
        ledState = HIGH;
      } else {
        ledState = LOW;
      }

      // set the LED with the ledState of the variable:
      digitalWrite(LED_BUILTIN, ledState);
    }
  }

bool getConfig ()
  {
    StaticJsonBuffer<200> jsonBuffer;
    Serial.println();
    Serial.println("Connecting to SD:");
    if (!SD.begin(4)) {
      Serial.println("SD Error: 1");
      return false;
    }
    Serial.println("SD Connected.");

    SDCon = SD.open(configFile);
    if (SDCon) {
      int i =0;
      while (SDCon.available()) {
        char c=SDCon.read();
        if (c=='}'){
          speedoSDData.concat(c);
          //Serial.println(speedoSDData);
            JsonObject& root = jsonBuffer.parseObject(speedoSDData);
            if (!root.success()) {
              Serial.println("parseObject() failed");
              return false;
            }
          
          distanceKM = root["distance"];
          Serial.print("Distance was set to: ");
          Serial.println(distanceKM);

        }
        speedoSDData.concat(c);
      }
      SDCon.close();
      return true;
    }
    else {
      Serial.println("File Error: 1");
      return false;
    }
  }
bool updateDistanceSD ()
  {
    StaticJsonBuffer<200> jsonBuffer;
    speedp = distanceKM;
    Serial.println("Connecting to SD:");
    if (!SD.begin(4)) {
      Serial.println("SD Error: 1");
      return false;
    }
    Serial.println("SD Connected.");
    SD.remove(configFile);

    SDCon = SD.open(configFile, FILE_WRITE);
    if (SDCon) {
      char buffer[200];
      JsonObject& dist = jsonBuffer.createObject();
      dist["distance"] = distanceKM;
      Serial.print("dst: ");
      Serial.println(distanceKM);
      Serial.println("___________");
      dist.prettyPrintTo(buffer);
      SDCon.print(buffer);
      SDCon.close();
      Serial.print("Updated dist: ");
      Serial.println(buffer);
      return true;
    }
    else {
      Serial.println("File Error: 1");
      return false;
    }
  }
void speedCalc ()
  { //Function called by the interrupt

    distanceKM = distanceKM + (wheelCirc/1000);
    lastRev = millis();

    if ( ( millis() - start ) > debounceRate )
      {
          elapsed = millis() - start;
          start = millis();
          speedk = ( 3600 * wheelCirc ) / elapsed;
      }
  }
void loop()
  {
    //set speed to zero if HALL Sensor open for more than 2s
    if ( millis() - lastRev > 2000 ){
      speedk = 0;
    }
    if ( updateSD && speedk != speedp &&  millis() - lastRefresh > 15000 ) {
      updateDistanceSD();    
      lastRefresh = millis();
    }
    Serial.print(int(speedk));
    Serial.println(" km/h");
    Serial.print(distanceKM);
    Serial.println(" km");
    delay(refreshRate);
    blink();
  }
