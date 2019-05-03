#include <Wire.h>
#include <SD.h>
#include <SPI.h>
#include <Adafruit_ADS1015.h>
#include "RTClib.h"

#define DEBUG 0 //set to 1 to get more Serial out statements
#define sd 1
//#define mux1  0x70 //defines the multiplexer address
#define num_ports 8 //defines how many ports are on the multiplexer
#define num_sensors 4 //defines how many sensors are in use
#define num_mux 1 //defines how many multiplexers are in use
#define card_select 10 //for SD card and datalogger
#define slope 19.775 //calculated slope for getting weight values from ADSD1115



DateTime now; //sets time
File data; //declares variable for datalogger
RTC_PCF8523 rtc; //initializes real time clock


struct state_ads1115{
  Adafruit_ADS1115 ads; //initializes ads1115
  bool is_setup = 0; //tells whether the struct has been setup
  float weight; //float for storing current weigt value
  float zero; //float for storing individual zero counts
};

byte addresses[num_mux] = {mux1}; //array for storing mux addresses

state_ads1115 sensors[num_sensors]; //struct array for ADS

void tcaselect(byte addrs, uint8_t i) {
  if (i > 7 or i < 0){
    Serial.println("Mux Address Out Of Bounds"); //error handling
    return;
  }
  if (addrs > 0x77 or addrs < 0x70){
    Serial.println("Address Out Of Bounds"); //error handling
    return;
  }
  Wire.beginTransmission(addrs); //communicates with the ports on the mux board
  Wire.write(1 << i); //initializes the port for communication
  Wire.endTransmission();  //turns on the port
}

void set_weight(state_ads1115& state_ads){
  float w = 0;
  float w_ave = 0;
  for(int i = 0; i < 10; i++){
    w = state_ads.ads.readADC_SingleEnded(0); //reads onboard ADS1115
    w = w-state_ads.zero; //subtracts tared zero value from read weight
    w = w/slope; //divides by calculated slope
    w_ave+=w;
  }
  w_ave = w_ave/10;
  state_ads.weight = w_ave; //sets struct weight to calculated weight
}

void setup_ads1115(state_ads1115& state_ads){
  state_ads.ads.begin(); //initializes onboard ADS1115
  state_ads.ads.setGain(GAIN_ONE); 
  state_ads.zero=tare(state_ads.ads); //runs the tare function for the individual state_ads
  #if DEBUG
   Serial.print("Zero in setup: ");
   Serial.println(state_ads.zero);
  #endif
  state_ads.is_setup=1;
}

float tare(Adafruit_ADS1115 ads){
   float z = 0; //float to store running total for averaging
  int16_t s = 0; //int to store current data value
  for(int i=0; i<1000; i++){
      s=ads.readADC_SingleEnded(0); //read current data value from ADS1115 
      z=z+s; //add current data value to running total for averaging
      #if DEBUG
        Serial.print("Value: ");    
        Serial.println(s);   // Serial out statements for testing/debug purposes 
        Serial.println(z);
      #endif
      delay(1); 
  }
  z=z/1000; //divide z by number of iterations to get average for zeroing out
 // Serial.print("Z= ");    //debug print statements
  //Serial.println(z);
  return z; //returns the calculated zero value
}

int count = 0; //variable for selecting from ads array

void setup() {
  Serial.begin(9600);
  #if sd
    while(!SD.begin(card_select)){
      Serial.println("Missing SD Card..."); //error handling for missing/broken SD card
      delay(100);
    }
  #endif
   Wire.begin(); //initializes I2C communication
 // while(!Serial)
  
  for(int i=0; i<num_mux; i++){
    for(int j=0; j<num_sensors; j++){
      tcaselect(addresses[i],j); //selects the multiplexer and port on multiplexer to gather data from
      setup_ads1115(sensors[count]); //sets up the sensor at selected port
      #if DEBUG
        Serial.print("Mux ");
        Serial.print(i);
        Serial.print(" port "); //
        Serial.print(j);
        Serial.println(" set up");
      #endif
      count++; //moves to the next sensor in the array
    }
  }
   Serial.println("Sensors Initialized");
}

void loop() {
  count = 0; 
  for(int i=0; i<num_mux; i++){
    for(int j=0; j<num_sensors; j++){
      tcaselect(addresses[i],j); //selects the multiplexer and port on multiplexer to gather data from
      set_weight(sensors[count]); //gathers weight data from sensor and sets weight value within struct

       count++;
    }
  }
    count = 0;
    //loop for displaying gathered data to the Serial monitor and the SD card
  for(int i=0; i<num_mux; i++){
    for(int j=0; j<num_sensors; j++){
     Serial.print("Weight ");
     Serial.print(count);
     Serial.print(": ");
     Serial.println(sensors[count].weight);
     count++;
     
    }
  }
  now=rtc.now();
  #if sd
    data = SD.open("data.txt",FILE_WRITE); //datalogging 
    count = 0;
      //prints time data onto sd card
      data.print("--");
      data.print(now.month());
      data.print('/');
      data.print(now.day());
      data.print('/');
      data.print(now.year());
      data.print(",");
      data.print(now.hour(), DEC);
      data.print(':');
      data.print(now.minute(), DEC);
      data.print(':');
      data.print(now.second(), DEC);
      data.println("--");
  for(int i=0; i<num_mux; i++){
    for(int j=0; j<num_sensors; j++){
      tcaselect(addresses[i],j);
      data.print("Evaporometer #");
      data.print(count+1); //prints evaporometer measurements onto sd card
      data.print(" weight: ");
      data.println(sensors[count].weight);
      count++;
    }
  }
  data.close();

/*
  //prints data in an excel friendly way
  data = SD.open("dataExport.txt",FILE_WRITE);
  count = 0;
      //prints time data onto sd card
      data.print(now.month());
      data.print('/');
      data.print(now.day());
      data.print('/');
      data.print(now.year());
      data.print(",");
      data.print(now.hour(), DEC);
      data.print(':');
      data.print(now.minute(), DEC);
      data.print(':');
      data.print(now.second(), DEC);
      data.print(",");
  for(int i=0; i<num_mux; i++){
    for(int j=0; j<num_sensors; j++){
      tcaselect(addresses[i],j);
      //prints evaporometer measurements onto sd card
      data.print(sensors[count].weight);
      count++;
    }
  }
  data.println();
  data.close();
  */
  #endif
  /*
  data.print("Evap 1 Weight: ");
  data.println((a.ads.readADC_SingleEnded(0)-a.zero)/19.775);
  
  data.close();

*/
 delay(2000);
}
