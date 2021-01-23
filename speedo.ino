/*
  Arduino speedometer
  @Author Yudish Appadoo
  @Email  yud.me@icloud.com
  @Repo   github.com/yudlab
 
*/
#include <SPI.h>
#include <SD.h>

int braudRate = 115200;
int debounceRate = 100; //in ms
int refreshRate = 1000; //in ms
float wheelCirc = 1.93; // wheel circumference (in meters)
uint8_t GPIO_Pin = D2; // Pin where the hall sensor is connected to
char odoPath = "yudlab/odo"; //file containing distance
File myFile;

float start, finished, distanceKM, speedk, lastRev;
int elapsed;

void ICACHE_RAM_ATTR speedCalc ();


void setup ()
  {

    Serial.begin(braudRate);
    while (!Serial) {
      ; /* wait for serial port to connect. Needed for native USB port only
      *** You would want to comment this if on PROD */
    }

    //toggle function speedCalc when HALL Sensor closes
    attachInterrupt(digitalPinToInterrupt(GPIO_Pin), speedCalc, RISING); // interrupt called when sensors sends digital 2 high (every wheel rotation)

    //start now (it will be reset by the interrupt after calculating revolution time)
    start = millis(); 

    // Print a transitory message to the LCD.
    Serial.print("Hello Yudish. :)");
    delay(5000); //just to allow you to read the initialization message
    distanceKM = 0; //@Todo: fetch distanceKM from SDCard
  }
 
void speedCalc ()
  { //Function called by the interrupt

    distanceKM = distanceKM + wheelCirc;
    lastRev = millis();

    if ( ( millis() - start ) > debounceRate )
      {
          //calculate elapsed
          elapsed = millis() - start;

          //reset start
          start = millis();
          
          //calculate speed in km/h
          speedk = ( 3600 * wheelCirc ) / elapsed;
      }
  }
 
void loop()
  {
    //set speed to zero if HALL Sensor open for more than 2s
    if(millis() - lastRev > 2000){
      speedk = 0;
    }
    Serial.print(int(speedk));
    Serial.print(" km/h \n");
    Serial.print(distanceKM);
    Serial.print(" km\n");
    delay(refreshRate); 
  }
