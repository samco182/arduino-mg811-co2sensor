/************************MG-811 Gas Sensor Module******************************
Author: Samuel Cornejo: sam.ach1990@gmail.com
Reference: Demo for MG-811 Gas Sensor Module V1.1 by Tiequan Shao: tiequan.shao@sandboxelectronics.com
************************************************************************************/

/*******************************Libraries*******************************************/
#include "Timer.h"

/************************Hardware Related Macros************************************/
#define         MG_PIN                       (0)     //define which analog input channel you are going to use
#define         BOOL_PIN                     (2)
#define         BUZZER_PIN                   (3)     //define buzzer output pin
#define         DC_GAIN                      (8.5)   //define the DC gain of amplifier

/***********************Software Related Macros************************************/
#define         READ_SAMPLE_INTERVAL         (50)    //define how many samples you are going to take in normal operation
#define         READ_SAMPLE_TIMES            (5)     //define the time interval(in milisecond) between each samples in 
                                                     //normal operation
#define         BUZZER_DELAY_TIME            (1500)  //buzzer delay time in milliseconds()

/**********************Application Related Macros**********************************/
//These two values differ from sensor to sensor. user should derermine this value.
#define         ZERO_POINT_VOLTAGE           (0.394) //define the output of the sensor in volts when the concentration of CO2 is 400PPM
#define         REACTION_VOLTGAE             (0.059) //define the voltage drop of the sensor when move the sensor from air into 1000ppm CO2

/*****************************Globals***********************************************/
float           CO2Curve[3]  =  {2.602,ZERO_POINT_VOLTAGE,(REACTION_VOLTGAE/(2.602-3))};   
                                                     //two points are taken from the curve. 
                                                     //with these two points, a line is formed which is
                                                     //"approximately equivalent" to the original curve.
                                                     //data format:{ x, y, slope}; point1: (lg400, 0.324), point2: (lg4000, 0.280) 
                                                     //slope = ( reaction voltage ) / (log400 –log1000) 
Timer           timer;
/**********************************************************************************/

void setup() {
    Serial.begin(9600);                              //UART setup, baudrate = 9600bps
    
    pinMode(BOOL_PIN, INPUT);                        //set pin to input
    digitalWrite(BOOL_PIN, HIGH);                    //turn on pullup resistors

    pinMode(BUZZER_PIN,OUTPUT);                      //set pin to output
    timer.every(BUZZER_DELAY_TIME, soundBuzzer);     //setup timer's every() function callback to be used

    Serial.print("MG-811 Demostration\n");                
}

void loop(){
    timer.update();

    int percentage;
    float volts;
   
    volts = MGRead(MG_PIN);
    Serial.print( "SEN0159:" );
    Serial.print(volts); 
    Serial.print( "V           " );
    
    percentage = MGGetPercentage(volts,CO2Curve);
    Serial.print("CO2:");
    if (percentage == -1) {
        Serial.print( "<400" );
    } else {
        Serial.print(percentage);
    }
    Serial.print( "ppm" );  
    Serial.print( "       Time point:" );
    Serial.print(millis());
    Serial.print("\n");
    
    if (digitalRead(BOOL_PIN) ){
        Serial.print( "=====BOOL is HIGH======" );
    } else {
        Serial.print( "=====BOOL is LOW======" );
    }
      
    Serial.print("\n");
    
    delay(200);
}

/*****************************  MGRead *********************************************
Input:   mg_pin - analog channel
Output:  output of SEN0159
Remarks: This function reads the output of SEN0159
************************************************************************************/  
float MGRead(int mg_pin) {
    int i;
    float v=0;

    for (i=0;i<READ_SAMPLE_TIMES;i++) {
        v += analogRead(mg_pin);
        delay(READ_SAMPLE_INTERVAL);
    }
    v = (v/READ_SAMPLE_TIMES) *5/1024 ;
    return v;  
}

/*****************************  MQGetPercentage ************************************
Input:   volts   - SEN0159 output measured in volts
         pcurve  - pointer to the curve of the target gas
Output:  ppm of the target gas
Remarks: By using the slope and a point of the line. The x(logarithmic value of ppm) 
         of the line could be derived if y(MG-811 output) is provided. As it is a 
         logarithmic coordinate, power of 10 is used to convert the result to non-logarithmic 
         value.
************************************************************************************/ 
int  MGGetPercentage(float volts, float *pcurve) {
   if ((volts/DC_GAIN )>=ZERO_POINT_VOLTAGE) {
      return -1;
   } else { 
      return pow(10, ((volts/DC_GAIN)-pcurve[1])/pcurve[2]+pcurve[0]);
   }
}

/******************************  soundBuzzer  **************************************
Input:   None
Output:  None
Remarks: Function to be called back by timer's every() function
************************************************************************************/ 
void soundBuzzer() {
  if (digitalRead(BOOL_PIN)) {
      tone(BUZZER_PIN, 1000, 500);
  }
}
