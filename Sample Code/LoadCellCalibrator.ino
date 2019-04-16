#include <Wire.h> //includes library for I2C communication
#include <Adafruit_ADS1015.h> //includes library for communication between main board and ADS1115

Adafruit_ADS1115 ads1115(0x48); // construct an ads1115 at address 0x49
const int sleep_pin = 13; //define the pin that controls the sleep setting of the board
const int num_cal = 4; //define the number of weights used for calibration
const float slope = 19.58; //Averaged rate of change gram by gram for load cell calibration
const float y_int = -534.4; //y intercept for line of best fit
const float w_cal[num_cal]={339.92,539.93,739.97,939.96}; //define the weights used for calibration
                                  //Weights are defined in grams, between 200g and 2000g

int16_t weights[num_cal]; //initialize the array to store the weight values for calibration

void setup(void)
{
  Serial.begin(9600); //initializes Serial monitor for user interface
  ads1115.begin();  // Initialize ads1115
  ads1115.setGain(GAIN_ONE);
  pinMode(sleep_pin, OUTPUT); //Initialize and turn on the amplifier
  digitalWrite(sleep_pin, LOW); //turns off amplifier
}

/*******************************************************
 * Prompts user to place on set weights to 
 * develop a line of best fit for calibrating load cell
 *******************************************************/
void getWeights(){
  int16_t adc0; //initialize a variable for storing read values from load cell to ADS1115
  for(int i=0; i<num_cal; i++){
      Serial.print("Add on ");
      Serial.print(w_cal[i]); //displays the weight value to add on to the load cell
      Serial.println("g weight and enter any key to continue");
     
    while(Serial.read()==-1){
           adc0 = ads1115.readADC_SingleEnded(0); //Wait until user has placed weight and inputs character
           delay(100);
    }
    Serial.print("Weight Value: "); //Serial output for user
    Serial.println(adc0);
    weights[i]=adc0; //adds the meausured value into the weight array for calibration
   
  }
}

/*************************************************
 * Prints the measured weight values in an easy 
 * to read way for copying over to microsoft excel
 ************************************************/
void printWeights(){
  for(int i=0; i<num_cal; i++){
      Serial.print(w_cal[i]);
      Serial.print("g Weight Val: ");
      Serial.println(weights[i]); 
  }
  Serial.println();
  Serial.println("Copy Data Into Spreadsheet");
  for(int i=0; i<num_cal; i++){
     Serial.print(w_cal[i]);
     Serial.print(",");
     Serial.println(weights[i]); //displays the measured weight values for transfering to
                                 //text file in order to develop rate of change per gram
  }
}

/*******************************************
 * Returns the expected measured value for
 * a weight based upon calibrated slope
 *******************************************/
float calculateCalibration(int val){
  float calc = (val*slope)+y_int; //returns the expected value for inputted weight
  return calc;
}

/**********************************************************
 * Creates and displays useful data for comparing
 * individual load cell meaurements to a calculated average
 **********************************************************/
void displayCalibration(){
  float error; //initialize variable for holding difference between measured and accepted values
  float percent_error; //initialize variable for holding percent error
  for(int i=0; i<num_cal; i++){ //loop through for the number of weights used for calibration
      Serial.print("Expected Value For ");
      Serial.print(w_cal[i]); //prints inputted weight
      Serial.print("g: ");
      float cal = calculateCalibration(w_cal[i]); //gets expected value
      Serial.print(cal); //prints expected value for inputted weight
      Serial.print("  Actual Value: "); 
      Serial.println(weights[i]); //prints actual measured value
      Serial.print("Error in values is ");
      error = (float(weights[i])-cal)/cal; 
      percent_error=error*100; //percent error calculation
      if(percent_error<0)
        percent_error=percent_error*-1; //accounts for negative error
      Serial.print(percent_error);
      Serial.println("%"); 
  }

  
}

void loop(void)
{
   while(!Serial); //wait for user to open Serial
   Serial.println("----Calibrate Load Cell----");
  digitalWrite(sleep_pin, HIGH); //turns on amplifier
  getWeights();  //prompt user to add weights
  printWeights(); //displays weights to copy into text file for immport into excell
  displayCalibration(); //displays how far off from accepted average individual load cells are
  digitalWrite(sleep_pin, LOW); //turns off amplifier
  delay(2000);
}
