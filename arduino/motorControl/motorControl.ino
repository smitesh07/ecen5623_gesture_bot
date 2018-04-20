// Adafruit Motor shield library
// copyright Adafruit Industries LLC, 2009
// this code is public domain, enjoy!

#include <AFMotor.h>

// Motor 1 is left motor 
// Motor 2 is right motor

AF_DCMotor motor1(1);
AF_DCMotor motor2(2);

void setup() {
  Serial.begin(9600);           // set up Serial library at 9600 bps
  Serial.println("Motor test!");

  // turn on motor
  motor1.setSpeed(200);
  motor2.setSpeed(200);
 
  motor1.run(RELEASE);
  motor2.run(RELEASE);
}

void loop() {
  uint8_t i;
  
  Serial.print("forward\n");
  
  motor1.run(FORWARD);
  motor2.run(FORWARD);
  for (i=0; i<255; i++) {
    motor1.setSpeed(i);  
    motor2.setSpeed(i);  
    delay(10);
 }
 
  for (i=255; i!=0; i--) {
    motor1.setSpeed(i);  
    motor2.setSpeed(i);  
    delay(10);
 }
  
  Serial.print("reverse \n");

  motor1.run(BACKWARD);
  motor2.run(BACKWARD);
  for (i=0; i<255; i++) {
    motor1.setSpeed(i);  
    motor2.setSpeed(i);  
    delay(10);
 }
 
  for (i=255; i!=0; i--) {
    motor1.setSpeed(i);  
    motor2.setSpeed(i);  
    delay(10);
 }
  

  Serial.print("release \n");
  motor1.run(RELEASE);
  delay(1000);
}
