#include <OneWire.h> 
#define SensorPin A0           //pH meter Analog output to Arduino Analog Input 0
#define Offset 3.07            //deviation compensation
#define LED 13
#define samplingInterval 20
#define printInterval 800
#define ArrayLenth  40          //times of collection
int pHArray[ArrayLenth];        //Store the average value of the sensor feedback
int pHArrayIndex=0;  

int DS18S20_Pin = 2;            //DS18S20 temperature sensor signal pin on digital 2

//Temperature chip i/o
OneWire ds(DS18S20_Pin);        //DS18S20 temperature sensor on digital pin 2

//pH Code
void setup(void)
{
  pinMode(LED,OUTPUT);          //
  Serial.begin(9600);  
  Serial.println("pH meter experiment!");    //Test the serial monitor
}
void loop(void)
{
  static unsigned long samplingTime = millis();
  static unsigned long printTime = millis();
  static float pHValue,voltage;
  float temperature;

  //Read pH into pH array based on sampling time and sampling interval
  //Also get temperature reading
  if(millis()-samplingTime > samplingInterval)
  {
      pHArray[pHArrayIndex++]=analogRead(SensorPin);
      if(pHArrayIndex==ArrayLenth)pHArrayIndex=0;
      voltage = avergearray(pHArray, ArrayLenth)*5.0/1024;    //To filter noise, the average of the array is used
      pHValue = 3.5*voltage+Offset;   //Convert voltage to pH, and incorporate the offset
      temperature = getTemp();
      samplingTime=millis();
  }

  //Print the voltage, pH, and temperature values to the serial
  if(millis() - printTime > printInterval)   //Every 800 milliseconds, print a numerical, convert the state of the LED indicator
  {
    Serial.print("Voltage:");   
    Serial.print(voltage,2);
    Serial.print("    pH value: ");
    Serial.println(pHValue,2);
    Serial.print("Temperature:");
    Serial.print(temperature,2);
    digitalWrite(LED,digitalRead(LED)^1);
    printTime=millis();
  }
    delay(800); //just here to slow down the output so it is easier to read
}

//----------------- FUNCTIONS --------------------------------

//Averages the voltage reading taken from the pH sensor to reduce error
//Returns an averaged pH value
double avergearray(int* arr, int number){
  int i;
  int max,min;
  double avg;
  long amount=0;
  
  //Error check
  if(number<=0){
    Serial.println("Error number for the array to avraging!/n");
    return 0;
  }
  
  //If number < 5, make no changes
  if(number<5){   //less than 5, calculated directly statistics
    for(i=0;i<number;i++){
      amount+=arr[i];
    }
    avg = amount/number;
    return avg;
  }
  
  else{
    if(arr[0]<arr[1]){
      min = arr[0];max=arr[1];
    }
    else{
      min=arr[1];max=arr[0];
    }
    for(i=2;i<number;i++){
      if(arr[i]<min){
        amount+=min;        //arr<min
        min=arr[i];
      }else {
        if(arr[i]>max){
          amount+=max;    //arr>max
          max=arr[i];
        }else{
          amount+=arr[i]; //min<=arr<=max
        }
      }//if
    }//for
    avg = (double)amount/(number-2);
  }//if
  
  return avg;
}



//Temperature Code
float getTemp(){
  //returns the temperature from one DS18S20 in DEG Celsius

  byte data[12];
  byte addr[8];

  if ( !ds.search(addr)) {
      //no more sensors on chain, reset search
      ds.reset_search();
      return -1000;
  }

  if ( OneWire::crc8( addr, 7) != addr[7]) {
      Serial.println("CRC is not valid!");
      return -1000;
  }

  if ( addr[0] != 0x10 && addr[0] != 0x28) {
      Serial.print("Device is not recognized");
      return -1000;
  }

  ds.reset();
  ds.select(addr);
  ds.write(0x44,1); // start conversion, with parasite power on at the end

  byte present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE); // Read Scratchpad

  
  for (int i = 0; i < 9; i++) { // we need 9 bytes
    data[i] = ds.read();
  }
  
  ds.reset_search();
  
  byte MSB = data[1];
  byte LSB = data[0];

  float tempRead = ((MSB << 8) | LSB); //using two's compliment
  float TemperatureSum = tempRead / 16;
  
  return TemperatureSum;
  
}
