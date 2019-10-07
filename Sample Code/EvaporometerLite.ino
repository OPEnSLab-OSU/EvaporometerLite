/******************************************************
 * Evaporometer Lite
 * Written By Elijah Shumway
 * 05/20/2019
 ******************************************************/
#include <Wire.h>
#include <SD.h>
#include <SPI.h>
#include <Adafruit_ADS1015.h>
#include "RTClib.h"
#include "Adafruit_SHT31.h"


/*---------------Define features of the board on or off--------------------*/
#define DEBUG 0 //set to 1 to get more Serial out statements
#define sd 1 //write 1 if using sd card dataloggger
#define SHT 0 //write 1 if using SHT temp/humidity sensor
/*-------------------------------------------------------------------------*/

/*-------------------------variable definitions-----------------------------*/
#define mux1  0x70 //defines the multiplexer address
#define num_ports 8 //defines how many ports are on the multiplexer
#define num_sensors 3 //defines how many sensors are in use
#define num_mux 1 //defines how many multiplexers are in use
#define card_select 10 //for SD card and datalogger
#define slope 19.775 //calculated slope for getting weight values from ADSD1115
#define delay_time 600000 //set time between measurements in milliseconds


/*------------------------Class Declarations for Sensors-----------------------------*/
DateTime now; //sets time
File data; //declares variable for datalogger
RTC_PCF8523 rtc; //initializes real time clock
Adafruit_SHT31 sht31 = Adafruit_SHT31();
/*-----------------------------------------------------------------------------------*/

/*------------------------Evaporometer Struct Declaration----------------------------*/
struct state_ads1115{
  Adafruit_ADS1115 ads; //initializes ads1115
  bool is_setup = 0; //tells whether the struct has been setup
  float weight; //float for storing current weigt value
  float zero; //float for storing individual zero counts
};
/*-----------------------------------------------------------------------------------*/


/*------------------------------Array Declarations-----------------------------------*/
byte addresses[num_mux] = {mux1}; //array for storing mux addresses

state_ads1115 sensors[num_sensors]; //struct array for ADS
/*------------------------------------------------------------------------------------*/

/*****************************************************************
 * tcaselect
 * Selects port on TCA multiplexer for communication
 * Inputs: TCA multiplexer address, int 0<i<7 for port selection
 * Outputs: Selects port on multiplexer to communicate with
 ****************************************************************/

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

/*************************************************************************
 * set_weight
 * Reads loadcell data from ads1115 and stores into state_ads1115 struct
 * Inputs: ads1115 struct from array
 * Outputs: weight value into state_ads1115 struct
 ************************************************************************/

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
/***********************************************************
 * setup_ads1115
 * sets up and tares the evaporometer load cell
 * Inputs: ads1115 struct from array
 * Outputs: initialized ads1115
 *********************************************************/
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
/*****************************************************************************************
 * tare
 * takes an average of 500 readings to calculate an initial average to tare loadcell
 * Inputs: Adafruit_ADS1115 from state_ads1115 struct
 * Outputs: Tares loadcell and returns the zero value for storing in state_ads1115 struct
 ****************************************************************************************/
float tare(Adafruit_ADS1115 ads){
   float z = 0; //float to store running total for averaging
  int16_t s = 0; //int to store current data value
  for(int i=0; i<500; i++){
      s=ads.readADC_SingleEnded(0); //read current data value from ADS1115 
      z=z+s; //add current data value to running total for averaging
      #if DEBUG
        Serial.print("Value: ");    
        Serial.println(s);   // Serial out statements for testing/debug purposes 
        Serial.println(z);
      #endif
      delay(1); 
  }
  z=z/500; //divide z by number of iterations to get average for zeroing out
 // Serial.print("Z= ");    //debug print statements
  //Serial.println(z);
  
  return z; //returns the calculated zero value
}

int count = 0; //variable for selecting from ads array

void setup() {
  pinMode(13,OUTPUT); //initialize pin for displaying errors
  
  #if SHT
  //Serial.println("SHT31 test");
  if (! sht31.begin(0x44)) {   //Initializes temperature/humidity sensor
    Serial.println("Couldn't find SHT31"); 
    digitalWrite(13,HIGH); //turns LED on if sensor cannot be initialized
    while (1) delay(1);
  }
  #endif
  
  Serial.begin(9600);
  #if sd
    while(!SD.begin(card_select)){
      Serial.println("Missing SD Card..."); //error handling for missing/broken SD card
      delay(100);
    }
    #if DEBUG
      Serial.println("SD initialized");
    #endif
            
  #endif
  
   Wire.begin(); //initializes I2C communication
 
  
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
  #if SHT
    float temp = sht31.readTemperature(); //reads SHT for temperature and humdity data
    float hmdty = sht31.readHumidity();
    #if DEBUG
      Serial.println(temp);
      Serial.println(hmdty);
    #endif
  #endif
  
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
  #if DEBUG
    Serial.println("RTC Set");
  #endif
  
  #if sd
    data = SD.open("evapdata.txt",FILE_WRITE); //datalogging
    digitalWrite(13,LOW); 
    if(data){
      count = 0;
        //prints time data onto sd card
        
        data.print(now.month());
        data.print('-');
        data.print(now.day());
        data.print('-');
        data.print(now.year());
        data.print(" ");
        data.print(now.hour(), DEC);
        data.print(':');
        data.print(now.minute(), DEC);
        data.print(':');
        data.print(now.second(), DEC);
       // data.print(",");
    for(int i=0; i<num_mux; i++){
      for(int j=0; j<num_sensors; j++){
        tcaselect(addresses[i],j);
        data.print(",");
        data.print(sensors[count].weight);
        count++;
      }
    }
        #if SHT //prints temperature and humdity data to SD card
          data.print(",");
          data.print(temp);
          data.print(",");
          data.print(hmdty);
        #endif
    data.println();
    
    #if DEBUG
      Serial.println("data in sd buffer");
    #endif
    
    data.close(); //writes data to sd card
    
    #if DEBUG
      Serial.println("data printed to SD card");
    #endif
    
    }
    else{
      Serial.println("Error opening file");
      digitalWrite(13,HIGH);
    }

  #endif
  
 delay(delay_time); //delays between readings
}
