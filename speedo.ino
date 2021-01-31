/*
  Arduino speedometer
  @Author Yudish Appadoo
  @Email  yud.me@icloud.com
  @Repo   github.com/yudlab

  LCD Connection:
  SCL - D1
  SDA - D2
  VCC - Vin - 5v
  GND - GND

  SD Connection:
  GND  - GND
  VCC  - 3.3v
  SCK  - D5
  MISO - D6
  MOSI - D7
  CS   - D8

*/
#include <SPI.h>
#include <SD.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

int braudRate = 9600;
int debounceRate = 300; //in ms
float wheelCirc = 0.00193; // wheel circumference (in km)

uint8_t HALL_SENSOR = D4; // Pin where the hall sensor is connected to
uint8_t CLEAR_ODO_BTN =  D3; //Pin to reset odo
const char* configFile = "config.odo"; //file containing the configuration settings

LiquidCrystal_I2C lcd(0x27, 16, 2);

String speedoSDData;
DynamicJsonBuffer jsonBuffer;
File SDCon;
char buffer[200];
int ledState = LOW; // ledState used to set the LED
unsigned long previousMillis = 0; // will store last time LED was updated
const long blinkInterval = 700; // ms
bool updateSD = false;

float previousSpeed, start, currentSpeed, tLastRev, tLastRefresh, tLastReset;
int tElapsed;
unsigned long odo, trip;

void ICACHE_RAM_ATTR counter ();
void ICACHE_RAM_ATTR resetOdo ();

void setup ()
  {
    Serial.begin(braudRate);
    while (!Serial) {
      ; /* wait for serial port to connect. Needed for native USB port only
      *** You would want to comment this if on PROD */
    }
    Wire.begin(D2, D1);
    //toggle function counter when HALL Sensor closes
    attachInterrupt(digitalPinToInterrupt(HALL_SENSOR), counter, RISING);
    attachInterrupt(digitalPinToInterrupt(CLEAR_ODO_BTN), resetOdo, RISING);
    pinMode(LED_BUILTIN, OUTPUT);
    lcd.init();
    lcd.backlight();
    lcd.home();
    lcd.print('Initializing...');
    Serial.println();

    if(!getConfig()) {
      odo = 0;
      trip= 0;
      blink(1000);
      lcd.setCursor(0, 0);
      lcd.print("SD Card Error");
      delay(1000);
    } else {
      updateSD = true;
    }

    //start now (it will be reset by the interrupt after calculating revolution time)
    start = millis(); 

    // Print a transitory message to the LCD.
    lcd.home();
    lcd.print("Hello Yudish!");
    delay(3000); //just to allow you to read the initialization message
    previousSpeed = 0.01;
  }
void loop()
  {
    if(previousSpeed != currentSpeed) {
      previousSpeed = currentSpeed;
      lcd.clear();
      lcd.home();
      lcd.print(currentSpeed);
      lcd.setCursor(4, 0);
      lcd.print(" km/h   ODO");
      lcd.setCursor(0, 1);
      lcd.print("TRP:");
      lcd.setCursor(4, 1);
      lcd.print(trip);
      lcd.setCursor(11, 1);
      lcd.print(odo);
    }
    
    //set speed to zero if HALL Sensor open for more than 2s
    if ( millis() - tLastRev > 2000 ){
      currentSpeed = 0;
    }
    if ( updateSD && ( currentSpeed != previousSpeed && ( millis() - tLastRefresh ) > 15000 ) ) {
      updateDistanceSD();    
      tLastRefresh = millis();
    }

    
    delay(1100);
  }
bool getConfig ()
  {
    Serial.println();
    Serial.println("Connecting to SD(getConfig):");
    if (!SD.begin(D8)) {
      Serial.println("SD Connection error(state D8)");
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
void counter ()
  { //Function called by the interrupt
    tLastRev = millis();
    if ( ( millis() - start ) > debounceRate )
    {
        tElapsed = millis() - start;
        start = millis();
        currentSpeed = ( 3600000 * wheelCirc ) / tElapsed;
    }

    odo = odo + wheelCirc;
    trip = trip + wheelCirc;

    blink(blinkInterval);
  }
void resetOdo ()
  {
    if(trip != 0 && millis() - tLastReset > 3000 ){
      trip = 0;
      updateDistanceSD();
      tLastReset = millis();
      lcd.clear();
      lcd.print("ODO RESET!");
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
