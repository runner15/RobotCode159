#include <QTRSensors.h>

#define NUM_SENSORS             8  // number of sensors used
#define NUM_SAMPLES_PER_SENSOR  4  // average 4 analog samples per sensor reading
#define EMITTER_PIN             2  // emitter is controlled by digital pin 2
//Docs: https://www.pololu.com/docs/0J19/all
QTRSensorsAnalog qtra((unsigned char[]) {0, 1, 2, 3, 4, 5, 6, 7}, 
NUM_SENSORS, NUM_SAMPLES_PER_SENSOR, EMITTER_PIN);
unsigned int sensorValues[NUM_SENSORS];
unsigned int line_position=0; // value from 0-7000 to indicate position of line between sensor 0 - 7

// ArduMoto motor driver vars
// pwm_a/b sets speed.  Value range is 0-255.  For example, if you set the speed at 100 then 100/255 = 39% duty cycle = slow
// dir_a/b sets direction.  LOW is Forward, HIGH is Reverse 
int pwm_a = 10;  //PWM control for Ardumoto outputs A1 and A2 is on digital pin 10  (Left motor)
int pwm_b = 11;  //PWM control for Ardumoto outputs B3 and B4 is on digital pin 11  (Right motor)
int dir_a = 12;  //direction control for Ardumoto outputs A1 and A2 is on digital pin 12  (Left motor)
int dir_b = 13;  //direction control for Ardumoto outputs B3 and B4 is on digital pin 13  (Right motor)

// motor tuning vars 
int calSpeed = 165;   // tune value motors will run while auto calibration sweeping turn over line (0-255)

// Proportional Control loop vars
float error=0;
float PV =0 ;  // Process Variable value calculated to adjust speeds and keep on line
float kp = 1;  // This is the Proportional value. Tune this value to affect follow_line performance
int m1Speed=0; // (Left motor)
int m2Speed=0; // (Right motor)

void setup() { // put your setup code here, to run once:
  //Set control pins to be outputs
  pinMode(pwm_a, OUTPUT);  
  pinMode(pwm_b, OUTPUT);
  pinMode(dir_a, OUTPUT);  
  pinMode(dir_b, OUTPUT);  

  //set both motors to stop
  analogWrite(pwm_a, 0);  
  analogWrite(pwm_b, 0);
  // then start calibration phase and move the sensors over both
  // reflectance extremes they will encounter in your application:
  for (int i = 0; i < 250; i++)  // make the calibration take about 5 seconds
  {
    qtra.calibrate();
    delay(20);
  }
  
}

void loop() { // put your main code here, to run repeatedly:
  // read calibrated sensor values + obtain measure of line position from 0 to 9000
  unsigned int position = qtra.readLine(sensorValues);
}
