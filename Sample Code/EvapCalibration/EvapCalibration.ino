#include <Wire.h>
#include <Adafruit_ADS1015.h>

Adafruit_ADS1115 ads1115(0x48);

int samples = 20; //set how many samples are taken for averaging
void setup() {
  Serial.begin(9600);
  ads1115.begin(); //initialize ADC on Evap board
  ads1115.setGain(GAIN_ONE); //sets built in ADC Gain
}

void loop() {
  int val = 0; //reset val to 0 after each average
  for(int i=0; i<samples; i++){
     val+=ads1115.readADC_SingleEnded(0); //reads value from ADC
  }
  val = val/samples;  //averages value 
  Serial.println(val); //prints value to terminal
  delay(250);
}
