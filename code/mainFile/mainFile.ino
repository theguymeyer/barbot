// Author: Guy Meyer
// Date of creation: May 18th 2020
// Last change date: May 18th 2020
// Objective: accept user input (serial) and execute the drink combination
// System Description:
//	- Hardware: 
//      IR Sensor
//      200 step NEMA 17 Bipolar Stepper providing linear motion underneat bottles
//      12VDC Pneumatic Valve + 2-way piston used to dispense drink + 200 PSI compressor (set to 40 PSI)
//
//	- Electrical: 
//      9VDC stable power. 
//      The RESET button is electrically connected to the power of the board and will perform a hard reset (ie. reinitialization).
//      IR senses proximity and outputs 5V bollean value
//
//	- Software: 
//			INPUT: a serial message indicating the drink requirements based on 5 unique bottle
//			serial message structure: uint8{Bottle_1, Bottle_2, Bottle_3, Bottle_4, Bottle_5} @ BAUDRATE=9600 via RX/TX serial bus
//			where each Bottle is a byte describing the number of shots from each bottle in ASCII. The system is not concerned with speed optimization with the serial buffer
//
//      OUTPUT: 
//      Electrical signals to stepper based on Bottle. 
//      Stops stepper if reached endpoint
//      Toggles solenoid via electrical signals
//      System timing
//      Serial reply when completed
//      `
//
// Design: 
//      On init -> INIT state 
//        - the system will write init_code to serial buffer indicating that its ready to serve
//        - the system will reset its end-effector -> first move piston down, then reset rail to starting position
//      USER_INPUT state
//        - detected new message in serial buffer
//        - Parse serial message to build drink list
//      PATH_PLANNER state
//        - Build a plan for the stepper's motion
//      MAKE_DRINK state
//        LOOP: move to bottle -> pour shot (x number of shots) -> if more bottle repeat, else break
//      SERVE_DRINK state
//        - the system will reset its end-effector -> first move piston down, then reset rail to starting position
//        - the system will send finished drink code to UI
//
//
//      __________                       _____________                      _______________                   _______________                   _______________  
//  o-->|  INIT   |       -->            | USER_INPUT |       -->           | PATH_PLANNER |       -->        |  MAKE_DRINK  |       -->        | SERVE_DRINK |
//      |_________|  SERIAL DETECTED     |____________|   PARSED MESSAGE    |______________|  DEFINED PLAN    |______________|  EXECUTED PATH   |_____________|
//                                            /\                                                                                                       |
//                                             |_______________________________________________________________________________________________________|
//                                                                                                SERIAL DETECTED (new drink)
//
//
//

#include <Servo.h> 
#include <Wire.h>
#include <Adafruit_MotorShield.h>
#include "utility/Adafruit_MS_PWMServoDriver.h"

// Create the motor shield object with the default I2C address
Adafruit_MotorShield AFMS = Adafruit_MotorShield(); 
// Or, create it with a different I2C address (say for stacking)
// Adafruit_MotorShield AFMS = Adafruit_MotorShield(0x61); 

// Connect a stepper motor with 200 steps per revolution (1.8 degree)
// to motor port #2 (M3 and M4)
Adafruit_StepperMotor *myMotor = AFMS.getStepper(200, 1);
 
Servo myservo;  // create servo object to control a servo 
                // twelve servo objects can be created on most boards

// init variables
int QRE1113_Pin = 2; //connected to digital 2
bool servo_pos = LOW;    // variable to store the servo position 
int stepper_pos = 0;
int count = 0;
int servo_max_up = 15;
int servo_max_down = 110;

// bottle positions
int start = 0;
int bottle1 = 240;
int bottle2 = 700;
int bottle3 = 1140;
int bottle4 = 1600;
int bottle5 = 2130;
int bottle6 = 2580;
int max_rail = 2800;

int drinks[][4] = {{bottle1, bottle2, bottle3, bottle2},
                   {bottle4, bottle2, bottle4, bottle6},
                   {bottle1, bottle1, bottle5, bottle2}};

int desired_drink = 0;

void servoDown()
{ /* lowers the servo to its neutral position */
  myservo.write(servo_max_down);  // initial location
  servo_pos = LOW;      // translates to physically Low (horizontal)
}

void servoUp()
{ /* raises the servo to its dispensing position */
  myservo.write(servo_max_up);  // initial location
  servo_pos = HIGH;     // translates to physically High (vertical)
}

void takeShot()
{ /* Raises and lowers the servo */
  int shotTime = 5000;
  
  //Serial.print("Dispensing Liquid");
  //Serial.println();
    
  servoUp();
  delay(shotTime);
  servoDown();
}

void resetRail()
{ /* sets the servo and stepper to its starting position */
  while(true) {
    int QRE_Value = digitalRead(QRE1113_Pin);
    if (QRE_Value == 1) {
      myMotor->step(1, FORWARD, DOUBLE);
    } else {
      break; 
    }
    delay(1);
  }
  
  stepper_pos = 0;
  
  //Serial.println("Reached Start!");
  
  servoUp();
  //Serial.println("Insert Cup, Ready for Drink!");
  
  return;
}

void moveTo(int bottle_num)
{ /* Moves to a desired bottle/position */

  /*
  Serial.print("Moving to, ");
  Serial.print(bottle_num);
  Serial.println();
  */
  
  int steps_needed;
  
  if (stepper_pos < bottle_num) {  // Move BACKWARD (away from start)
    steps_needed = bottle_num - stepper_pos;
    myMotor->step(steps_needed, BACKWARD, DOUBLE);
    
  } else if (stepper_pos > bottle_num) {  // Move FORWARD (towards start)
    steps_needed = stepper_pos - bottle_num;
    myMotor->step(steps_needed, FORWARD, DOUBLE);
    
  } else {  // at desired position
    delay(2000);  // fill up time
  }
  
  stepper_pos = bottle_num;
  
  return;
}

void setup() 
{ 
  
  Serial.begin(9600);  // initiates serial communication
  
  /*while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }*/
  
  myservo.attach(10);  // attaches the servo on pin 9 to the servo object 
  
  delay(1000);  // gives the servo time to settle
  
  // Serial.println("Returning to starting position...");

  AFMS.begin();  // create with the default frequency 1.6KHz
  //AFMS.begin(1000);  // OR with a different frequency, say 1KHz
  
  myMotor->setSpeed(400);  // 10 rpm 
  pinMode( QRE1113_Pin, INPUT );
  
  // needs a reset to get started
  for (int i = 0; i < 3; i ++){
    myservo.write(10);
    delay(1000);  
    myservo.write(0);
    delay(1000);
  }
  
  resetRail();
} 

void loop() 
{
  if (Serial.available()) {
    desired_drink = Serial.read();
    //Serial.print(desired_drink);
    int shots_count = sizeof(drinks[desired_drink])/2;
    
    //Serial.println(shots_count);
    
    servoDown();
    for (int i = 0; i < shots_count; i++)
    {
      moveTo(drinks[desired_drink][i]);
      delay(100);
      takeShot();
      delay(100);
    }
    
    resetRail();  // move to start
    /*Serial.println();
    Serial.println("Enjoy your drink!");
    Serial.println();*/
    
    Serial.write("0");
    
  }

  servoUp();
} 


