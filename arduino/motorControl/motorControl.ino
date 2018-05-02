// Adafruit Motor shield library
// copyright Adafruit Industries LLC, 2009
// this code is public domain, enjoy!

#include <AFMotor.h>

#define delay_val 200

AF_DCMotor left(4);
AF_DCMotor right(3);

void setup() {
  
  
  Serial.begin(115200);           // set up Serial library at 9600 bps
  //Serial.println("Startup \n");

  // turn on motor
  left.setSpeed(255);
  right.setSpeed(255);
 
  left.run(RELEASE);
  right.run(RELEASE);
}

int received_char;

void loop() {

  if (Serial.available() > 0) {
        // read the incoming byte:
        received_char = Serial.read();

        #ifdef DEBUG
        Serial.print("Got- ");
        Serial.println(received_char, DEC);
        #endif
    }

    switch (received_char) 
    {
      case 49:
        #ifdef DEBUG
        Serial.print("Forward\n\r");
        #endif
        left.run(FORWARD);
        right.run(FORWARD);
        delay(delay_val); 
        Serial.write('f');
        break;
      case 50:
        #ifdef DEBUG
        Serial.print("Reverse\n\r");
        #endif
        left.run(BACKWARD);
        right.run(BACKWARD);
        delay(delay_val);
        Serial.write('b');
        break;
      case 51:
        #ifdef DEBUG
        Serial.print("Left\n\r");
        #endif
        left.run(BACKWARD);
        right.run(FORWARD);
        delay(delay_val);
        Serial.write('l');
        break;
      case 52:
        #ifdef DEBUG
        Serial.print("Right\n\r");
        #endif
        left.run(FORWARD);
        right.run(BACKWARD);
        delay(delay_val);
        Serial.write('r');
        break;
      case 53:
        left.run(RELEASE);
        right.run(RELEASE);
        delay(delay_val);
        Serial.write('s');
      default:
        left.run(RELEASE);
        right.run(RELEASE);
        delay(delay_val);
        break;
    }

  received_char=0;
}
