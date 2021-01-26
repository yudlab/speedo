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
int debounceRate = 300; //in ms
float wheelCirc = 0.00193; // wheel circumference (in km)

uint8_t HALL_SENSOR = D2; // Pin where the hall sensor is connected to
uint8_t CLEAR_ODO_BTN =  D1; //Pin to reset odo
const char* configFile = "config.odo"; //file containing the configuration settings

String speedoSDData;
DynamicJsonBuffer jsonBuffer;
File SDCon;
char buffer[200];

int ledState = LOW; // ledState used to set the LED
unsigned long previousMillis = 0; // will store last time LED was updated
const long blinkInterval = 700; // ms
bool updateSD = false;


float odo, trip, previousSpeed, start, currentSpeed, tLastRev, tLastRefresh, tLastReset;
int tElapsed;


void ICACHE_RAM_ATTR speedCalc ();
void ICACHE_RAM_ATTR resetOdo ();

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
      odo = 0;
      trip= 0;
      blink(blinkInterval);
    } else {
      updateSD = true;
    }
    //toggle function speedCalc when HALL Sensor closes
    attachInterrupt(digitalPinToInterrupt(HALL_SENSOR), speedCalc, RISING);
    attachInterrupt(digitalPinToInterrupt(CLEAR_ODO_BTN), resetOdo, RISING);

    //start now (it will be reset by the interrupt after calculating revolution time)
    start = millis(); 

    // Print a transitory message to the LCD.
    Serial.println("Hello Yudish.");
    delay(3000); //just to allow you to read the initialization message
  }
void loop()
  {
    //set speed to zero if HALL Sensor open for more than 2s
    if ( millis() - tLastRev > 2000 ){
      currentSpeed = 0;
    }
    if ( updateSD && currentSpeed != previousSpeed &&  millis() - tLastRefresh > 10000 ) {
      updateDistanceSD();    
      tLastRefresh = millis();
    }

    
    Serial.println();
    Serial.print(currentSpeed);
    Serial.print("KM/H  | TRIP: ");
    Serial.print(trip);
    Serial.print("KM | ODO: ");
    Serial.print(odo);
    Serial.print("KM");
    Serial.println();
    delay(900);
  }
bool getConfig ()
  {
    Serial.println();
    Serial.println("Connecting to SD(getConfig):");
    if (!SD.begin(4)) {
      Serial.println("SD Connection error(state 4)");
      return false;
    }
    Serial.println("SD Connected.");

    SDCon = SD.open(configFile);
    if (SDCon) {
      int i = 0;
      while (SDCon.available()) {
        char c=SDCon.read();
        if (c=='}'){
          speedoSDData.concat(c);
          JsonObject& root = jsonBuffer.parseObject(speedoSDData);
          if (!root.success()) {
            Serial.println("JSON INVALID/PARSE ERROR");
            Serial.println("Contents: ");
            Serial.println(speedoSDData);
            return false;
          }
          odo = root["odo"];
          trip = root["trip"];
          Serial.print("ODO: ");
          Serial.println(odo);
          Serial.print("TRIP: ");
          Serial.println(trip);
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
    previousSpeed = currentSpeed;
    SD.remove(configFile);

    SDCon = SD.open(configFile, FILE_WRITE);
    if (SDCon) {
      DynamicJsonBuffer jsonBuffer;
      JsonObject& dist = jsonBuffer.createObject();
      if (!dist.success()) {
        Serial.println("ERROR CREATING JSON");
        return false;
      }

      dist["odo"] = odo;
      dist["trip"] = trip;
      dist.prettyPrintTo(buffer);
      SDCon.print(buffer);
      Serial.println("SD Updated: ");
      Serial.println(buffer);
      SDCon.close();
      return true;
    }
    else {
      Serial.println("File Error: 1");
      return false;
    }
  }
void speedCalc ()
  { //Function called by the interrupt
    blink(blinkInterval);
    tLastRev = millis();
    if ( ( millis() - start ) > debounceRate )
    {
        tElapsed = millis() - start;
        start = millis();
        currentSpeed = ( 3600000 * wheelCirc ) / tElapsed;
    }

    odo = odo + wheelCirc;
    trip = trip + wheelCirc;

  }
void resetOdo ()
  {
    if(trip != 0 && millis() - tLastReset > 3000 ){
      trip = 0;
      updateDistanceSD();
      tLastReset = millis();
    }
  }
int blink (int blinkInterval)
  {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= blinkInterval) {
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
