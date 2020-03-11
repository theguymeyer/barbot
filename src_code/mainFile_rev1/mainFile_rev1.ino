/* 
Author: Guy Meyer
Last Edited: March 11 2020

Main file for bartender robot
  This controller is responsible for moving the cup and dispensing the liquid
  It listens for commands from the UI controller outlining its drink requirements
  At initialization 'resetRail()' is called to ensure that the platform is ready to accept a cup
  
  INPUT:
    Serial connection to via jumper to pin D0 & D1
    IR sensor - linear origin for stepper motor (calibration should happen often)
    
  OUTPUT:
    Serial - writes ASCII "F" to serial bus to denote that a drink is ready to be picked up by user
    Adafruit Motor Shield:
      Linear Motion - Bipolar NEMA 17 Stepper, 200 steps per Revolution (connected to M1 + M2 connector)
    Liquid Dispensing - Pneumatic Piston controller with 2-position 12V Solenoid Valve (SV) operating at approx 3 Bar pressure
        -> The SV must be actuated in both direction and doesnt include spring mechanism
        -> SV actuated using relay switch (triggered on falling edge - ie. no current flow when set to HIGH (5V))
        -> Piston actuated approx 8cm stroke to reach outsourced dispensing mechanism pouring out approx 1.5 liquid oz

Arduino Pinouts:
D0 - Serial Rx
D1 - Serial Tx
D2 - IR signal read
D3 - 
D4 - 
D5 - 
D6 - SV position 2 (piston DOWN)
D7 - SV position 1 (piston UP)
D8 - 
D9 - 
D10 - 
D11 - 
D12 - 
D13 - 
A1 - 
A2 - 
A3 - 
A4 - 
A5 - 
5V - IR sensor + Relay Switch Board + Adafruit Shield
GND - IR sensor + Relay Switch Board + Adafruit Shield
Vin - 
*/ 

//#include <Servo.h> 
#include <Wire.h>
#include <Adafruit_MotorShield.h>
#include "utility/Adafruit_MS_PWMServoDriver.h" // TODO: remove... might be useless ???

// Create the motor shield object with the default I2C address
Adafruit_MotorShield AFMS = Adafruit_MotorShield(); 
// Or, create it with a different I2C address (say for stacking)
// Adafruit_MotorShield AFMS = Adafruit_MotorShield(0x61); 

// Connect a stepper motor with 200 steps per revolution (1.8 degree)
// to motor port #1 (M1 and M2)
Adafruit_StepperMotor *myMotor = AFMS.getStepper(200, 1);
Adafruit_StepperMotor *myMotor2 = AFMS.getStepper(400, 2);

//Servo myservo;  // create servo object to control a servo 
                // twelve servo objects can be created on most boards

// piston state enumurated variable
enum PistonState {
  UP,  // raised and dispensing liquid
  DOWN  // lowered and does not dispense liquid
};

// init variables - TODO make these "#define" expressions
uint16_t motor_speed = 200;
uint16_t micro_step_delay = 700;
int ir_Pin = 2;  // IR sensor read pin
int shot_time = 4000;  // time to dispense shot - milliseconds
int refill_time = 2000;  // time for liquid dispenser to refill - milliseconds
int piston_UP_Pin = 7;
int piston_DOWN_Pin = 6;
int SV_waittime = 50;  // latch time to move solenoid valve to new position - milliseconds
int platform_pos;  // tracks platform position in number of steps

// initial piston state is down
PistonState piston_state = DOWN;

// bottle positions
short start = 0;
short bottle1 = 420;
short bottle2 = 950;
short bottle3 = 1400;
short bottle4 = 1890;
short bottle5 = 2400;
//int bottle6 = 2000; // remove (only 5 bottle)
short max_rail = 2500;

short drinks[][5] = {{bottle1, bottle4, bottle4, bottle1},
                   {bottle2, bottle2, bottle5, bottle5, bottle5},
                   {bottle3, bottle4, bottle4, bottle3}};

int desired_drink; 

/* Piston Control Functions */

void pistonUp() 
{
  digitalWrite(piston_DOWN_Pin, HIGH);  // ensure no uncertainty in position of SV
  digitalWrite(piston_UP_Pin, LOW);
  delay(SV_waittime);
  digitalWrite(piston_UP_Pin, HIGH);  // Piston will remain in UP position with pneumatic power
  
  piston_state = UP;
}

void pistonDown() 
{
  digitalWrite(piston_UP_Pin, HIGH);  // ensure no uncertainty in position of SV
  digitalWrite(piston_DOWN_Pin, LOW);
  delay(SV_waittime);
  digitalWrite(piston_DOWN_Pin, HIGH);  // Piston will remain in DOWN position with pneumatic power
  
  piston_state = DOWN;
}

void takeShot()
{
  pistonUp();
  delay(shot_time);
  pistonDown();
  
  //Serial.print("Taking Shot");
}

void resetRail()
{ /* sets the servo and stepper to its starting position */

  int IR_Value;  // tmp variable

  while(true) {
    int IR_Value = digitalRead(ir_Pin);
    if (IR_Value == 1) {
      //myMotor->step(10, FORWARD, DOUBLE);
      myMotor->onestep(FORWARD, SINGLE);
      delayMicroseconds(micro_step_delay);  // advice from Bill from Adafruit
    } else {
      break; 
    }
    //delay(1);
  }
  
  platform_pos = 0;
  
  // allow motor to move freely
  myMotor->release();
}

void moveToStart()
{
  // prepare system
  pistonDown();
  resetRail();
}

void moveTo(int bottle_num)
{ // Moves to a desired bottle/position

  // TODO delete after testing and intergration with UI
  /*
  Serial.print("Moving to, ");
  Serial.print(bottle_num);
  Serial.println();
  */
  
  int steps_needed;  // tmp variable
  
  if (platform_pos < bottle_num) {  // Move BACKWARD (away from start)
    steps_needed = bottle_num - platform_pos;
    
    // testing of new motor speeds 
    //quickMove(steps_needed, BACKWARD);
    
    // This line currently works but really slow
    myMotor->step(steps_needed, BACKWARD, DOUBLE);
    
  } else if (platform_pos > bottle_num) {  // Move FORWARD (towards start)
    steps_needed = platform_pos - bottle_num;
    myMotor->step(steps_needed, FORWARD, DOUBLE);
    
  } else {  // at desired position
    delay(refill_time);
  }
  
  /*
  Serial.print("Reached bottle, ");
  Serial.print(bottle_num);
  Serial.println();
  */
  
  platform_pos = bottle_num;
  
}

void setup() 
{ 
  
  Serial.begin(9600);  // initiates serial communication
  
  /*while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }*/
  
  delay(1000);  // gives the servo time to settle - TODO remove?

  AFMS.begin();  // create with the default frequency 1.6KHz
  //AFMS.begin(1000);  // OR with a different frequency, say 1KHz
  
  // From Adafruit support page: https://forums.adafruit.com/viewtopic.php?f=31&t=57041&start=15
  TWBR = ((F_CPU /400000l) - 16) / 2; // Change the i2c clock to 400KHz
  
  myMotor->setSpeed(motor_speed);  // 10 rpm ?
  pinMode(ir_Pin, INPUT);  // IR detect sensor
  pinMode(piston_UP_Pin, OUTPUT);
  pinMode(piston_DOWN_Pin, OUTPUT);
  
  // deactivate both relays
  digitalWrite(piston_UP_Pin, HIGH);
  digitalWrite(piston_DOWN_Pin, HIGH);
  
  moveToStart();
  
  // clear the serial buffer
  Serial.read();
  Serial.flush();
  
  delay(1000);
} 

void loop() 
{
  
  if (Serial.available()) {
    desired_drink = Serial.read();
    Serial.flush();
    Serial.flush();
    
    // need to convert ASCII to int
    desired_drink = desired_drink - '0';
    
    // calculate number of stops - tracks the number of liquid shots required for current drink (used in for loop)
    uint16_t shots_count = sizeof(drinks[desired_drink])/2;  // why divided by 2?
    
    moveToStart();
    
    delay(1000);
    
    for (int i = 0; i < shots_count; i++)
    {
      moveTo(drinks[desired_drink][i]);
      delay(200);
      takeShot();
      delay(1000);
    }
    
    // return cup to user
    moveToStart();  // move to start
    
    Serial.write("F");  // code denotes that drink is ready
    
  }

} 


