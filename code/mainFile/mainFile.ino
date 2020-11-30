// Author: Guy Meyer
// Date of creation: May 18th 2020
// Last change date: June 8th 2020
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
//         /                                     /                                  /
//      _\/_______                            _\/__________                      _\/____________                   _______________                   _______________  
//  o-->|  INIT   |          -->              | USER_INPUT |       -->           | PATH_PLANNER |       -->        |  MAKE_DRINK  |       -->        | SERVE_DRINK |
//      |_________|  SERIAL INIT DETECTED     |____________|   PARSED MESSAGE    |______________|  DEFINED PLAN    |______________|  EXECUTED PATH   |_____________|
//                                                /\                                                                                                       |
//                                                 |_______________________________________________________________________________________________________|
//                                                                                                SERIAL DETECTED (new drink)
//
//
//

#include <Adafruit_MotorShield.h>
#include <Vector.h>

#define DRINK_COUNT 5
#define IR_THRESHOLD 0.9

#define BOTTLE1_POS 500   // in number fo steps from start
#define BOTTLE2_POS 1500
#define BOTTLE3_POS 2500
#define BOTTLE4_POS 3500
#define BOTTLE5_POS 4500
#define RAIL_MAX 5500

// TESTING
//#define BOTTLE1_POS 100   // in number fo steps from start
//#define BOTTLE2_POS 200
//#define BOTTLE3_POS 300
//#define BOTTLE4_POS 400
//#define BOTTLE5_POS 500
//#define RAIL_MAX 1000

#define DRINK_HOLD_TIME 3000
#define DRINK_INBETWEEN_TIME 2500

//using namespace std;

// init variables
int LimitSW_Pin = 2;    // connected to digital 2
int pistonUp_Pin = 6;   // pneumatic controls
int pistonDown_Pin = 7;
int bottle_position [5] = {BOTTLE1_POS, BOTTLE2_POS, BOTTLE3_POS, BOTTLE4_POS, BOTTLE5_POS};

Adafruit_MotorShield AFMS = Adafruit_MotorShield(); 
Adafruit_StepperMotor *myMotor = AFMS.getStepper(200, 1);


// state variables
int pmsg [DRINK_COUNT] = {0};  // parsed message
int path [DRINK_COUNT][2] = {0}; // servo path
int stop_count = 0;

/* --- Custom Functions --- */

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

// moves the drink closer to the IR (starting position) with incremental steps
// -> return true on successful motion (returns false otherwise)
bool resetRail()
{
  pistonDown(); 
  delay(2000);
  
  while(!(isStepperAtStart())) {
    myMotor->step(10, FORWARD, DOUBLE);  // move closer
    delay(2); // give the motor time to move
  }

  // reached start
  pistonUp();
  return true;

}

bool isStepperAtStart()
{
  
  if (digitalRead(LimitSW_Pin)) {
    return true;
  }

  return false;
}

/* --- State Functions (executed on entry unless otherwise noted) --- */

// system init - runs on boot
// -> returns true when serial is established and the rail is reset (returns false otherwise)
bool init_sys()
{

  String ACK;
  if (Serial.available()) {
    ACK = Serial.readString();  // ACK receive

    if (ACK == "OK\n") {  // transition criteria

      resetRail();

      Serial.flush();
      Serial.flush();
      return true;
    }
  }

  return false;

}

// a waiting state for serial input regarding drink info
// updates the message buffer from serial stream (requirements for each drink) starting from closest
void userInput(int msg_buffer[])
{

  String message = Serial.readString();

  if (message != NULL) {  // if message in buffer
    for (int i = 0; i < DRINK_COUNT; i++) {
      msg_buffer[i] = message[i] - '0';
    }
  }

  // clean up
  Serial.flush();
  Serial.flush();

}

// Takes the user input and builds a path for the servo as a list of steps with directions
// returns a list of instructions for the stepper (2d vector) [direction, steps, shots]
// direction defined as 0 - FORWARD (closer to start), 1 - BACKWARD (away from start) 
// RETURNS the number of stops (ie. number of different requested drinks, the volume of each does not matter)
int pathPlanner(int msg_buffer[], int path_buffer[][2])
{
  // Note this function can be later optimized for better path planning
  
//  while (!isStepperAtStart()) { resetRail(); }

  int instruction [2] = {0,0};
  int path_index = 0;
  for (int d = 0; d < DRINK_COUNT; d++) {

    if (msg_buffer[d] != 0) {
      path_buffer[path_index][0] = bottle_position[d];
      path_buffer[path_index][1] = msg_buffer[d];
      path_index ++;
    }

  }

  return path_index;

}

// Execution of path - moves cup through positions and pours liquor
// Assumption: only requested liquors are in the path buffer
void exec(int path_buf[][2], int stop_count)
{

  resetRail();
  pistonDown();
  delay(1000);

  int loc = 50;
  uint8_t dir;
  
  myMotor->step(loc, BACKWARD, DOUBLE);

  for (int i = 0; i < stop_count; i++) {

    // move stepper to position until within margin of error - 10 steps
    while (abs(path_buf[i][0] - loc) > 1) {
      
      if (isStepperAtStart() || loc > RAIL_MAX) { 
        pistonDown();
        resetRail();
        return ; 
      }
      
      if (path_buf[i][0] > loc) {
        myMotor->step(1, BACKWARD, DOUBLE);
        loc++;
      } else {
        myMotor->step(1, FORWARD, DOUBLE);
        loc--;
      }

//      delay(2); // give the motor time to move
    }

    for (int j = 0; j < path_buf[i][1]; j++) {

      delay(DRINK_INBETWEEN_TIME);
      pistonUp();
      delay(DRINK_HOLD_TIME);
      pistonDown();
      delay(DRINK_INBETWEEN_TIME);

    }

    
    delay(300);

  }

  // return home
  resetRail();
  delay(100);
  myMotor->release();
  
}

void setup() 
{ 
  Serial.begin(9600); // init serial - no direct imports

  while (!Serial)
  {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  AFMS.begin();  // create with the default frequency 1.6KHz
  myMotor->setSpeed(1000);  // 10 rpm 
  
  pinMode(LimitSW_Pin, INPUT);
  pinMode(pistonUp_Pin, OUTPUT);
  pinMode(pistonDown_Pin, OUTPUT);

//  myMotor->step(10, BACKWARD, DOUBLE);
  //myMotor->release();

  while ( !(init_sys()) ) {}  // run init until it works and system is ready to go (return true)
  
} 

void loop() 
{

  // wait for new input
  if (Serial.available()) {
    userInput(pmsg);
    
    // path planner
    stop_count = pathPlanner(pmsg, path);
    
    // exec (move servo and actuate piston)
    exec(path, stop_count);
    
    // return home

    Serial.print("DONE");
    delay(1000);
    Serial.flush();
    Serial.flush();

  }

  // wait for new input
  delay(50);

  
  //Serial.println("thinking");

}
