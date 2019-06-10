#include <Wire.h>

#define card_select 10 //for SD card and datalogger
#define DATA_PIN 0
const int BREAK_IN = A1;

int RawValue= 0;
float Voltage = 0;


void setup(){
  Serial.begin(9600);
  pinMode(DATA_PIN, INPUT);
  pinMode(BREAK_IN, INPUT);
  analogReference(AR_EXTERNAL);
}

void loop(){
  RawValue = analogRead(BREAK_IN); 
  int16_t val=analogRead(DATA_PIN);
  Serial.print("Pin reads  ");
  Serial.println(val);
  delay(1000);

  RawValue = analogRead(BREAK_IN); 
  Voltage = (RawValue * 4.096 )/ 1024.0; // Notice 4.096 Here
  Serial.print("LM4040 reads  " );  // shows ADC Output                    
  Serial.println(RawValue);      


}
