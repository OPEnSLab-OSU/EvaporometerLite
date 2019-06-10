/****************************************************************************************
*Code for testing load cells to ensure that calculated weight matches true added weight
****************************************************************************************/

#include <Wire.h> //included for I2C communication
#include <Adafruit_ADS1015.h> //includes ADS communication for reading loadcell data

Adafruit_ADS1115 ads1115(0x48); // construct an ads1115 at address 0x48
const int sleep_pin = 13; //define the pin that controls the sleep setting of the board
const float slope = 19.58; //constant defined slope 

float zero = 0; //global defined variable for storing the value to zero out loadcell

/*****************************************************************
 * Calculates the value for zeroing out the evaporometer at start
 ****************************************************************/
float get_zero(){
  float z = 0; //float to store running total for averaging
  int16_t s = 0; //int to store current data value
  for(int i=0; i<1000; i++){
      s=ads1115.readADC_SingleEnded(0); //read current data value from ADS1115 
      z=z+s; //add current data value to running total for averaging
     /* Serial.print("Value: ");    
      Serial.println(s);    Serial out statements for testing/debug purposes 
      Serial.println(z);*/
      delay(1); 
  }
  z=z/1000; //divide z by number of iterations to get average for zeroing out
  return z; //return the zero value 
}

void setup(void)
{
 // Serial.begin(9600); //Initializes the Serial monitor for printing data out
  ads1115.begin();  // Initialize ads1115
  ads1115.setGain(GAIN_ONE);
  pinMode(sleep_pin, OUTPUT); //declare the sleep pin for turning on and off INA amplifier on PCB
  digitalWrite(sleep_pin, HIGH); //Turn on amplifier
  zero=get_zero(); //Set zero value for zeroing out loadcell
  Serial.println(zero);// Serial output for testingdebug purposes: displays the zero value calculated
}
/*******************************************************************************
 * Reads the ADS I2C and calculates the weight value using the calculated zero
 *******************************************************************************/
float getWeight(){
  float weight = 0; //float variable to store measured weight value
  weight=ads1115.readADC_SingleEnded(0)-zero; //gets ADS reading and subtracts the zero value to calculate weight
  weight=weight/slope; //divides the calculated value by the slope of the loadcell to calculate the true weight of objects in the evaporometer
  return weight; //returns the true weight value
}

void loop(){
  digitalWrite(sleep_pin,HIGH); //Turn ADS on --> comment out digital write statements to keep ADS on permenently
  Serial.println(getWeight()); //Prints out the calculated weight value to the Serial monitor
  digitalWrite(sleep_pin,LOW); //Turn ADS off --> comment out digital write statements to keep ADS on permenently
  delay(1000);
}
