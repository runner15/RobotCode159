#include <MotorDriver.h>
#include <QTRSensors.h>

#define NUM_SENSORS             5  // number of sensors used
#define NUM_SAMPLES_PER_SENSOR  4  // average 4 analog samples per sensor reading
#define EMITTER_PIN             2  // emitter is controlled by digital pin 2
//Docs: https://www.pololu.com/docs/0J19/all
#define SERVO_PIN 0
QTRSensorsAnalog qtra((unsigned char[]) {A0, A1, A2, A3, A5}, 
NUM_SENSORS, NUM_SAMPLES_PER_SENSOR, EMITTER_PIN);
unsigned int sensorValues[NUM_SENSORS];
unsigned int line_position=0; 
// value from 0-4000 to indicate position of line between sensor 0-4

MotorDriver motor;
MotorDriver motor1;

// Proportional Control loop vars
float error=0;
float PV=0 ;  // Process Variable value calculated to adjust speeds and keep on line
int m1Speed=0; // (Left motor)
int m2Speed=0; // (Right motor)

int turnCount = 0;
int turnLeft = 0;

// Servo
int servoSide = 1;
int servoRing1  = 6;
int servoRing2  = 7;
int servoShoot1 = 4;
int servoShoot2 = 5;

int startup = 0;
int startpin = 2;

void setup() { // put your setup code here, to run once:
  motor.begin();
  motor1.begin();

  // start calibration phase and move the sensors over both
  // reflectance extremes they will encounter in your application:
  for (int i = 0; i < 250; i++)  // make the calibration take about 5 seconds
  {
    qtra.calibrate();
    //delay(20);
  }

  //Servo pin modes to send signal to second UNO
  pinMode(servoRing1, OUTPUT);
  pinMode(servoRing2, OUTPUT);
  pinMode(servoShoot1, OUTPUT);
  pinMode(servoShoot2, OUTPUT);
  pinMode(startpin, INPUT);

  delay(250);
} //End setup

void loop() { // put your main code here, to run repeatedly:
  int starting = digitalRead(startpin);
  if (startup == 2)
  {
    // read calibrated sensor values + obtain 
    //measure of line position from 0 to 4000
    line_position = qtra.readLine(sensorValues);
    // begin line
    follow_line(line_position);
  }
  else if(starting == 1)
  {
    startup = 1;
  }
  else if (startup == 1)
  {
    digitalWrite(servoShoot1, HIGH);
    digitalWrite(servoShoot2, HIGH);
    delay(1000);
    digitalWrite(servoShoot1, LOW);
    digitalWrite(servoShoot2, LOW);
  
    motor.speed(0, -90);  // RIGHT MOTOR from back
    motor.speed(1, 75);
    delay(1000);
    startup = 2;  
  }
} //End main loop


// line following function
void follow_line(int line_position) //follow the line
{
  // 0 is far Right sensor while 4 (4000 return) is far Left sensor

  switch(line_position)
  {
       
    // Line has moved off the left edge of sensor
    // This will make it turn fast to the left
    case 4000:
           motor.speed(0, -100);        // RIGHT MOTOR from back
           motor.speed(1, 1);           // LEFT MOTOR from back
    break;

    // Line had moved off the right edge of sensor
    // This will make it turn fast to the right
    case 0:     
        motor.speed(0, -1);            // set motor0 to speed 100
        motor.speed(1, 100);
    break;
 
    // The line is still within the sensors. 
    // This will calculate adjusting speed to keep the line in center.
    default:      
      PV = (float)line_position - 2000; 
      // 2000 is center measure of 4000 far left and 0 on far right
  
      // this section limits the PV (motor speed pwm value)  
      // limit PV to 55
      if (PV >= 2000) 
      {
        PV = 80;
      }
  
      if (PV < 2000) 
      {
        PV = -80;
      }
      
      // Note that if PV > 0 the robot needs to turn left
      m1Speed = 0 - PV;
      m2Speed = 0 + PV;
     
      //set motor speeds
      motor.speed(0, m2Speed);            // RIGHT MOTOR from back
      motor.speed(1, m1Speed);
      break;
  } 
  
  bool lightLine = ((sensorValues[0] < 150)&&(sensorValues[1] < 150) && (sensorValues[2] < 150) && (sensorValues[3] < 150) && (sensorValues[4] < 150));
  bool allDark = ((sensorValues[0] > 500)&&(sensorValues[1] > 500) && (sensorValues[2] > 500) && (sensorValues[3] > 500) && (sensorValues[4] > 500));
  if (lightLine)
  {
     if (turnCount == 2 || turnCount == 4)
     {
      delay(120);
     }
    delay(100);
    motor.brake(0);
    motor1.brake(1);
    turnCount = turnCount+1;
    servoSide=1;
    if (turnCount != 4 && turnCount != 7)
    {
      move_servo(servoSide);
    }
    delay(1000);
    int uturn = 0;
    while (lightLine) //Turn Code
    {
      if (turnCount==4)
      {
        
        if (uturn == 0)
        {
          motor1.speed(1,-75); // LEFT MOTOR from back
          motor.speed(0,-98); // RIGHT MOTOR from back
        }
        else if (uturn == 1)
        {
          motor.speed(0,-90);
          motor.speed(1,70);
          delay(500);
          motor.speed(0,-100); // RIGHT MOTOR from back
          delay(100);
          uturn = uturn + 1;
        }
        line_position = qtra.readLine(sensorValues);
        bool darkLine = ((sensorValues[3] > 250) || (sensorValues[4] > 250));
        if (darkLine && uturn == 0)
        {
          uturn = 1;
        }
        else if (darkLine && uturn == 2)
        {
           break;
        }
      }
      else
      {
        motor1.speed(1,80);
        motor.speed(0,70);
    
        line_position = qtra.readLine(sensorValues);
        bool darkLine = ((sensorValues[3] > 250) || (sensorValues[4] > 250));
        if (darkLine)
        {
          motor.brake(0);
          motor1.brake(1);
          servoSide=2;
          if (turnCount != 3 && turnCount != 5 && turnCount != 6)
          {
            move_servo(servoSide);
          }
          break;
        }
      }
    } 
  }
  /*if (allDark && turnCount > 4 && turnLeft == 0)
  {
    delay(100);
    motor.brake(0);
    motor1.brake(1);
    turnLeft = 1;
    while(true)
    {
      motor1.speed(1,-70);
      motor.speed(0,-80);
  
      line_position = qtra.readLine(sensorValues);
      bool darkLine = ((sensorValues[3] > 250) || (sensorValues[4] > 250));
      if (darkLine)
      {
        break;
      }
    }
  }*/

  if (turnCount == 5)
  {
    PV = (float)line_position - 2000;
    if (PV >= 2000) 
    {
      PV = 70;
    }
    if (PV < 2000) 
    {
      PV = -70;
    }
    m1Speed = 0 - PV;
    m2Speed = 0 + PV;
   
    motor.speed(0, m2Speed);            // RIGHT MOTOR from back
    motor.speed(1, m1Speed);
    if (allDark)
    {
      delay(3000);
      motor.brake(0);
      motor1.brake(1);
      turnLeft = 1;
      delay(100);
      motor1.speed(1,-80);
      motor.speed(0,-100);
      delay(1000);
      while(true)
      {
        line_position = qtra.readLine(sensorValues);
        bool darkLine = ((sensorValues[0] > 250) || (sensorValues[1] > 250));
        if (darkLine)
        {
          motor.brake(0);
          motor1.brake(1);
          turnCount = turnCount+1;
          //servoSide=2;
          break;
        }
      }
    }
  }
  if (turnCount > 7)
  {   
    motor.speed(0, -90);  // RIGHT MOTOR from back
    motor.speed(1, 80);
    delay(6500);
    motor.stop(0);
    motor1.stop(1);
  }

} // end follow_line

void move_servo(int servoSide) {
  if(servoSide == 2) {  
    digitalWrite(servoRing2, HIGH);
    delay(1000);
    digitalWrite(servoRing2, LOW);
    delay(1000);
  }
  else if(servoSide == 1) {
    digitalWrite(servoRing1, HIGH);
    delay(1000);
    digitalWrite(servoRing1, LOW);
  }  
}

