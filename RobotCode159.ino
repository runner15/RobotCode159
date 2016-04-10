#include <MotorDriver.h>
#include <QTRSensors.h>

#define NUM_SENSORS             6  // number of sensors used
#define NUM_SAMPLES_PER_SENSOR  4  // average 4 analog samples per sensor reading
#define EMITTER_PIN             2  // emitter is controlled by digital pin 2
//Docs: https://www.pololu.com/docs/0J19/all
#define SERVO_PIN 0
QTRSensorsAnalog qtra((unsigned char[]) {A0, A1, A2, A3, A4, A5}, 
NUM_SENSORS, NUM_SAMPLES_PER_SENSOR, EMITTER_PIN);
unsigned int sensorValues[NUM_SENSORS];
unsigned int line_position=0; // value from 0-7000 to indicate position of line between sensor 0 - 7

MotorDriver motor;
MotorDriver motor1;

// Proportional Control loop vars
float error=0;
float PV =0 ;  // Process Variable value calculated to adjust speeds and keep on line
float kp = 1;  // This is the Proportional value. Tune this value to affect follow_line performance
int m1Speed=0; // (Left motor)
int m2Speed=0; // (Right motor)

// Servo
// This is the time since the last rising edge in units of 0.5us.
uint16_t volatile servoTime = 0;
// This is the pulse width we want in units of 0.5us.
uint16_t volatile servoHighTime = 3000;
// This is true if the servo pin is currently high.
boolean volatile servoHigh = false;

void setup() { // put your setup code here, to run once:
  Serial.begin(9600);
  motor.begin();
  motor1.begin();
  servoInit();

  // start calibration phase and move the sensors over both
  // reflectance extremes they will encounter in your application:
  for (int i = 0; i < 250; i++)  // make the calibration take about 5 seconds
  {
    qtra.calibrate();
    //delay(20);
  }
  
  //Serial.begin(9600);
  for (int i = 0; i < NUM_SENSORS; i++)
  {
    Serial.print(qtra.calibratedMinimumOn[i]);
    Serial.print(' ');
  }
  Serial.println();
  delay(250);
  
} //End setup


void loop() { // put your main code here, to run repeatedly:
  // read calibrated sensor values + obtain measure of line position from 0 to 5000
  line_position = qtra.readLine(sensorValues);
  
  for (unsigned char i = 0; i < NUM_SENSORS; i++)
  {
    Serial.print(sensorValues[i]);
    Serial.print('\t');
  }
  //Serial.println(); // uncomment this line if you are using raw values
  Serial.println(line_position);
  
  // begin line
  follow_line(line_position);
} //End main loop


// line following function
//  Proportional Control Only
void follow_line(int line_position) //follow the line
{

  // 0 is far Right sensor while 5 (5000 return) is far Left sensor

  switch(line_position)
  {
       
    // Line has moved off the left edge of sensor
    // This will make it turn fast to the left
    case 5000:
           motor.speed(0, -100);            // RIGHT MOTOR from back
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
      error = (float)line_position - 2500; // 2500 is center measure of 5000 far left and 0 on far right
 
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
      if (PV >= 2500)
      {
        PV = 80;
      }
  
      if (PV < 2500)
      {
        PV = -80;
      }
      
      // adjust motor speeds to correct the path
      // Note that if PV > 0 the robot needs to turn left
      m1Speed = 0 - PV;
      m2Speed = 0 + PV;
     
      //set motor speeds
      motor.speed(0, m2Speed);            // RIGHT MOTOR from back
      motor.speed(1, m1Speed);
      break;
  } 

  bool lightLine = ((sensorValues[0] < 150) && (sensorValues[1] < 150) && (sensorValues[2] < 150) && (sensorValues[3] < 150) && (sensorValues[4] < 150) &&  (sensorValues[5] < 150));
  if (lightLine)
  {
    delay(100);
     motor.brake(0);
     motor1.brake(1);
     move_servo();
     delay(1000);
     while (lightLine) //Turn Code
     {
        motor1.speed(1,80);
        motor.speed(0,70);
        Serial.print("Turn Code");
        Serial.println();

        for (unsigned char i = 0; i < NUM_SENSORS; i++)
  {
    Serial.print(sensorValues[i]);
    Serial.print('\t');
  }
  //Serial.println(); // uncomment this line if you are using raw values
  Serial.println(line_position);
  
        line_position = qtra.readLine(sensorValues);
        bool darkLine = ((sensorValues[3] > 250) || (sensorValues[4] > 250));
        if (darkLine)
        {
          Serial.print("Line Catch Code");
          Serial.println();
          break;
        }
     } 
  }

} // end follow_line

void move_servo() {
  delay(1000); 
  servoSetPosition(1000);  // Send 1000us pulses.
  delay(1000);  
  servoSetPosition(1400);  // Send 2000us pulses.
  delay(1000);
}

// This ISR runs after Timer 2 reaches OCR2A and resets.
// In this ISR, we set OCR2A in order to schedule when the next
// interrupt will happen.
// Generally we will set OCR2A to 255 so that we have an
// interrupt every 128 us, but the first two interrupt intervals
// after the rising edge will be smaller so we can achieve
// the desired pulse width.
ISR(TIMER2_COMPA_vect)
{
  // The time that passed since the last interrupt is OCR2A + 1
  // because the timer value will equal OCR2A before going to 0.
  servoTime += OCR2A + 1;
   
  static uint16_t highTimeCopy = 3000;
  static uint8_t interruptCount = 0;
   
  if(servoHigh)
  {
    if(++interruptCount == 2)
    {
      OCR2A = 255;
    }
 
    // The servo pin is currently high.
    // Check to see if is time for a falling edge.
    // Note: We could == instead of >=.
    if(servoTime >= highTimeCopy)
    {
      // The pin has been high enough, so do a falling edge.
      digitalWrite(SERVO_PIN, LOW);
      servoHigh = false;
      interruptCount = 0;
    }
  } 
  else
  {
    // The servo pin is currently low.
     
    if(servoTime >= 40000)
    {
      // We've hit the end of the period (20 ms),
      // so do a rising edge.
      highTimeCopy = servoHighTime;
      digitalWrite(SERVO_PIN, HIGH);
      servoHigh = true;
      servoTime = 0;
      interruptCount = 0;
      OCR2A = ((highTimeCopy % 256) + 256)/2 - 1;
    }
  }
}
 
void servoInit()
{
  digitalWrite(SERVO_PIN, LOW);
  pinMode(SERVO_PIN, OUTPUT);
   
  // Turn on CTC mode.  Timer 2 will count up to OCR2A, then
  // reset to 0 and cause an interrupt.
  TCCR2A = (1 << WGM21);
  // Set a 1:8 prescaler.  This gives us 0.5us resolution.
  TCCR2B = (1 << CS21);
   
  // Put the timer in a good default state.
  TCNT2 = 0;
  OCR2A = 255;
   
  TIMSK2 |= (1 << OCIE2A);  // Enable timer compare interrupt.
  sei();   // Enable interrupts.
}
 
void servoSetPosition(uint16_t highTimeMicroseconds)
{
  TIMSK2 &= ~(1 << OCIE2A); // disable timer compare interrupt
  servoHighTime = highTimeMicroseconds * 2;
  TIMSK2 |= (1 << OCIE2A); // enable timer compare interrupt
}
