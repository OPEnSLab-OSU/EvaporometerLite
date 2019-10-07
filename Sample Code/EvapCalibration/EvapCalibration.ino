#include <Wire.h>
#include <Adafruit_ADS1015.h>

#define CALIBRATE 1 //define as 1 to run in calibration mode
#define CHECK 0     //define as 1 to run in check mode to verify calibration

#define slope 0     //set to calculated slope from linear regression
#define zero 0      //set to calculated zero from linear regression

Adafruit_ADS1115 ads1115(0x48);

int samples = 20; //set how many samples are taken for averaging
void setup() {
  Serial.begin(9600);
  ads1115.begin(); //initialize ADC on Evap board
  ads1115.setGain(GAIN_ONE); //sets built in ADC Gain
}

void loop() {
  #ifdef CALIBRATE
    int val = 0; //reset val to 0 after each average
    for(int i=0; i<samples; i++){
       val+=ads1115.readADC_SingleEnded(0); //reads value from ADC
    }
    val = val/samples;  //averages measured value 
    Serial.println(val); //prints value to terminal
    delay(250);
  #endif

  #ifdef CHECK
    int digital_weight = 0;
    int check_val = 0;
    for(int i=0; i<samples; i++) 
      check_val+=ads1115.readADC_SingleEnded(0); //reads value from ADC
    
    check_val = check_val/samples; //averages measured value
    
    digital_weight*=slope;
    digital_weight-=zero;
    
    Serial.println(digital_weight);
    delay(250);
  #endif
}
