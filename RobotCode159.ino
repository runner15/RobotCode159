#include <MotorDriver.h>
#include <seeed_pwm.h>
#include <VarSpeedServo.h> 

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

#define servoRightPin A1
#define servoLeftPin A0

// motor tuning vars 
int calSpeed = 165;   // tune value motors will run while auto calibration sweeping turn over line (0-255)

// Proportional Control loop vars
float error=0;
float PV =0 ;  // Process Variable value calculated to adjust speeds and keep on line
float kp = 1;  // This is the Proportional value. Tune this value to affect follow_line performance
int m1Speed=0; // (Left motor)
int m2Speed=0; // (Right motor)

// Servo
VarSpeedServo servoRight;    // create servo object to control a servo 
VarSpeedServo servoLeft;
unsigned int servoTotal = 1;

void setup() { // put your setup code here, to run once:
  //Set control pins to be outputs
  pinMode(pwm_a, OUTPUT);  
  pinMode(pwm_b, OUTPUT);
  pinMode(dir_a, OUTPUT);  
  pinMode(dir_b, OUTPUT);  

  servoRight.attach(servoRightPin);
  servoLeft.attach(servoLeftPin); 

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
  
} //End setup


void loop() { // put your main code here, to run repeatedly:
  // read calibrated sensor values + obtain measure of line position from 0 to 7000
  line_position = qtra.readLine(sensorValues);
  
  // begin line
  follow_line(line_position);
} //End main loop


// line following function
//  Proportional Control Only
void follow_line(int line_position) //follow the line
{

  // 0 is far Right sensor while 7 (7000 return) is far Left sensor

  switch(line_position)
  {
       
    // Line has moved off the left edge of sensor
    // This will make it turn fast to the left
    case 7000:
           digitalWrite(dir_a, HIGH); 
           analogWrite(pwm_a, 200);
           digitalWrite(dir_b, LOW);  
           analogWrite(pwm_b, 200);
    break;

    // Line had moved off the right edge of sensor
    // This will make it turn fast to the right
    case 0:     
           digitalWrite(dir_a, LOW); 
           analogWrite(pwm_a, 200);
           digitalWrite(dir_b, HIGH);  
           analogWrite(pwm_b, 200);
    break;
 
    // The line is still within the sensors. 
    // This will calculate adjusting speed to keep the line in center.
    default:      
      error = (float)line_position - 3500; // 3500 is center measure of 7000 far left and 0 on far right
 
      // This sets the motor speed based on a proportional only formula.
      // kp is the floating-point proportional constant you need to tune. 
      // Maybe start with a kp value around 1.0, tuned in declared Proportional Control loop vars at the top of this code.
      // Note that it's very important you get your signs right, or else the
      // control loop will be unstable.
   
      // calculate the new Process Variable
      // this is the value that will be used to alter the speeds
      PV = kp * error;
  
      // this section limits the PV (motor speed pwm value)  
      // limit PV to 55
      if (PV > 55)
      {
        PV = 55;
      }
  
      if (PV < -55)
      {
        PV = -55;
      }
      
      // adjust motor speeds to correct the path
      // Note that if PV > 0 the robot needs to turn left
      m1Speed = 200 - PV;
      m2Speed = 200 + PV;
     
      //set motor speeds
      digitalWrite(dir_a, LOW);  
      analogWrite(pwm_a, m1Speed);
      digitalWrite(dir_b, LOW);  
      analogWrite(pwm_b, m2Speed);
      break;
  } 

} // end follow_line 

/*void move_servo(int servoTotal) {
  ServoRight.write(125, 30, true); //back
  servoRight.write(90, 30, true); //push out
  if (servoTotal == 2) //Run the inside servo
  {
    servoLeft.write(8, 30, true); //back
    servoLeft.write(45, 30, true); //push out
  }
}*/

