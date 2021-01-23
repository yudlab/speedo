/*
  Arduino speedometer
  @Author Yudish Appadoo
  @Email  yud.me@icloud.com
  @Repo   github.com/yudlab
 
*/

int braudRate = 115200;
int debounceRate = 100; //in ms
float circMetric = 2.093; // wheel circumference (in meters)
uint8_t GPIO_Pin = D2; // Pin where the hall sensor is connected to

float start, finished;
float elapsed;
float speedk;


void ICACHE_RAM_ATTR speedCalc ();

void setup ()
{

  Serial.begin(braudRate);
  while (!Serial) {
    ; /* wait for serial port to connect. Needed for native USB port only
    *** You would want to comment this if on PROD */
  }

  attachInterrupt(digitalPinToInterrupt(GPIO_Pin), speedCalc, RISING); // interrupt called when sensors sends digital 2 high (every wheel rotation)

  //start now (it will be reset by the interrupt after calculating revolution time)
  start = millis(); 

  // Print a transitory message to the LCD.
  Serial.print("Hello Yudish. :)");
  delay(5000); //just to allow you to read the initialization message

}
 
void speedCalc ()
{
  //Function called by the interrupt

  if ((millis()-start)>debounceRate)
    {
        //calculate elapsed
        elapsed=millis()-start;

        //reset start
        start=millis();
  
        //calculate speed in km/h
        speedk=(3600*circMetric)/elapsed; 

    }
}
 
void loop()
{

  Serial.print(int(speedk));
  Serial.print(" km/h \n");

  delay(1000); 
}
