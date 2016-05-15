#include <Servo.h>

// Servo
Servo ring1; 
Servo ring2; 
Servo shoot1; 
Servo shoot2; 

int servoRing1  = 6;
int servoRing2  = 7;
int servoShoot1 = 4;
int servoShoot2 = 5;

void setup() {
  ring1.attach(A0);
  ring2.attach(A1);
  shoot1.attach(A2);
  shoot2.attach(A3);

  pinMode(servoRing1, INPUT);
  pinMode(servoRing2, INPUT);
  pinMode(servoShoot1, INPUT);
  pinMode(servoShoot2, INPUT);
}

void loop() {
    int Bring1 = digitalRead(servoRing1);
    int Bring2 = digitalRead(servoRing2);
    int Bshoot2 = digitalRead(servoShoot2);

    if(Bshoot2 == 1)
    {
      for (int pos = 0; pos <= 92; pos += 1) { 
        shoot2.write(pos);              
        delay(15);                       
      }
      for (int pos = 180; pos >= 53; pos -= 1) { 
        shoot1.write(pos);              
        delay(15);                       
      }
    }
    if(Bring1 == 1)
    {
      for (int pos = 40; pos >= -15; pos -= 1) { 
        ring1.write(pos);              
        delay(15);                       
      }
      for (int pos = -15; pos <= 40; pos += 1) { 
        // goes from -15 degrees to 40 degrees
        ring1.write(pos);              
        delay(15);                       
      }
    }
    if(Bring2 == 1)
    {
      for (int pos = 10; pos <= 60; pos += 1) { 
        ring2.write(pos);              
        delay(15);                      
      }
      for (int pos = 60; pos >= 10; pos -= 1) { 
        ring2.write(pos);              
        delay(15);                       
      }
    }
}
