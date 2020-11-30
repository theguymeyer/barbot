// Author: Guy Meyer
// Date: Nov 29 2020
// Objective: Use this file to accurately locate the position of the bottles


#include <Adafruit_MotorShield.h>

#define BOTTLE1_POS 500   // in number fo steps from start
#define BOTTLE2_POS 1500
#define BOTTLE3_POS 2500
#define BOTTLE4_POS 3500
#define BOTTLE5_POS 4500
#define RAIL_MAX 5500

int bottle_position [5] = {BOTTLE1_POS, BOTTLE2_POS, BOTTLE3_POS, BOTTLE4_POS, BOTTLE5_POS};
int location = 0;
int steps;
int LimitSW_Pin = 2; //connected to digital 
int pistonUp_Pin = 6;   // pneumatic controls
int pistonDown_Pin = 7;
String data;

Adafruit_MotorShield AFMS = Adafruit_MotorShield(); 
Adafruit_StepperMotor *myMotor = AFMS.getStepper(200, 1);

// moves the drink closer to the IR (starting position) with incremental steps
// -> return true on successful motion (returns false otherwise)
bool resetRail()
{
  
  while(!(isStepperAtStart())) {
    myMotor->step(10, FORWARD, DOUBLE);  // move closer
    delay(2); // give the motor time to move
  }

  // reached start
  return true;

}

bool isStepperAtStart()
{
  
  if (digitalRead(LimitSW_Pin)) {
    return true;
  }

  return false;
}

// alters the solenoid values to move the piston DOWN
bool pistonDown()
{
  digitalWrite(pistonUp_Pin, LOW);
  digitalWrite(pistonDown_Pin, HIGH);
}

// alters the solenoid values to move the piston UP
bool pistonUp()
{
  digitalWrite(pistonDown_Pin, LOW);
  digitalWrite(pistonUp_Pin, HIGH);
}

void softRelease()
{
//  digitalWrite(pistonDown_Pin, HIGH);
  digitalWrite(pistonDown_Pin, HIGH);

  delay(5);
  pistonUp();
  
}


void setup() {
  Serial.begin(9600); // init serial - no direct imports

  while (!Serial)
  {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  AFMS.begin();  // create with the default frequency 1.6KHz
  //AFMS.begin(1000);  // OR with a different frequency, say 1KHz
  
  myMotor->setSpeed(3000);  // 10 rpm 
  pinMode( LimitSW_Pin, INPUT );
  pinMode(pistonUp_Pin, OUTPUT);
  pinMode(pistonDown_Pin, OUTPUT);
  resetRail();

}

void loop() {
  

  if (Serial.available()) {

    data = Serial.readString();

    if (data == "r") { resetRail(); location = 0;}
    else if (data == "u") { pistonUp(); }
    else if (data == "d") { pistonDown(); }
    else if (data == "s") { softRelease(); }
    else {
      steps = data.toInt();
      location = location + steps;
      
      myMotor->step(steps, BACKWARD, DOUBLE);
    }

    Serial.print("The Current position is: ");
    Serial.println(location);
    myMotor->release();
  }
}
