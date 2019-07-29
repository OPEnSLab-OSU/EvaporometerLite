#include <Wire.h>
#include <SD.h>
#include <SPI.h>
#include <Adafruit_ADS1015.h>
#include "RTClib.h"
#include "Adafruit_SHT31.h"

#define card_select 10
#define delay_time 600000

#define slope 1
#define zero 1

DateTime now;
File data;
RTC_PCF8523 rtc; //initializes real time clock
Adafruit_SHT31 sht31 = Adafruit_SHT31();
Adafruit_ADS1115 ads(0x48);

void setup() {
  pinMode(A0,INPUT);
  pinMode(A1,INPUT);
  pinMode(A2,INPUT);
  Serial.begin(9600);
  ads.begin();
  ads.setGain(GAIN_ONE);
 // rtc.adjust(DateTime(F(__DATE__),F(__TIME__)));
   while(!SD.begin(card_select)){
      digitalWrite(13,HIGH);
      Serial.println("Missing SD Card..."); //error handling for missing/broken SD card
      
      delay(100);
      digitalWrite(13,LOW);
      delay(100);
    }
   if (! sht31.begin(0x44)) {   //Initializes temperature/humidity sensor
        Serial.println("Couldn't find SHT31"); 
        digitalWrite(13,HIGH); //turns LED on if sensor cannot be initialized
        while (1) delay(1);
  }
  // put your setup code here, to run once:
  Wire.begin();
}

void loop() {
  int val;
  float weight;
  int INA;
  int R1;
  int R2;
  
  val = ads.readADC_SingleEnded(0);
  weight = (val/slope)-zero;
  float temp = sht31.readTemperature();
  float hmdty = sht31.readHumidity();
  INA = analogRead(A0);
  R1 = analogRead(A1);
  R2 = analogRead(A2);
  
  now = rtc.now();
  Serial.print(now.month());
  Serial.print('-');
  Serial.print(now.day());
  Serial.print('-');
  Serial.print(now.year());
  Serial.print(' ');
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute()-7, DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.print(" Value: ");
  Serial.print(val);
  Serial.print("  Weight: ");
  Serial.print(weight);
  Serial.print("  Temperature: ");
  Serial.print(temp);
  Serial.print("  Humidity: ");
  Serial.print(hmdty);
  Serial.print("  INA Voltage: ");
  Serial.print(INA);
  Serial.print("  Resistor Side 1 Voltage: ");
  Serial.print(R1);
  Serial.print("  Resistor Side 2 Voltage: ");
  Serial.println(R2);
  data = SD.open("evapdata.txt",FILE_WRITE);
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
        data.print(",");
        data.print(weight);
        data.print(",");
        data.print(temp);
        data.print(",");
        data.print(hmdty);
        data.print(",");
        data.print(INA);
        data.print(",");
        data.print(R1);
        data.print(",");
        data.println(R2);
          
        data.close();

  delay(delay_time);
  // put your main code here, to run repeatedly:

}
