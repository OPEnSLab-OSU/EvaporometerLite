#include <Wire.h>
#include <Adafruit_ADS1015.h>

Adafruit_ADS1115 ads(0x49);

void setup(){
  Serial.begin(9600);
  ads.begin();
  ads.setGain(GAIN_ONE);
}

void loop(){
  int16_t val;
  val=ads.readADC_SingleEnded(0);
  Serial.println(val);
  delay(1000);
}
