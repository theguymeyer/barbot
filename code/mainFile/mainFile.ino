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

#define DRINK_HOLD_TIME 2500

using namespace std;

// init variables
int QRE1113_Pin = 2; //connected to digital 2
int bottle_position [5] = {BOTTLE1_POS, BOTTLE2_POS, BOTTLE3_POS, BOTTLE4_POS, BOTTLE5_POS};

Adafruit_MotorShield AFMS = Adafruit_MotorShield(); 
Adafruit_StepperMotor *myMotor = AFMS.getStepper(200, 2);


// state variables
int pmsg [DRINK_COUNT] = {0};  // parsed message
int path [DRINK_COUNT][2] = {0}; // servo path

/* --- Custom Functions --- */

// alters the solenoid values to move the piston DOWN
bool pistonDown()
{
  // TODO trigger relay for piston motion DOWN
}

// alters the solenoid values to move the piston UP
bool pistonUp()
{
  // TODO trigger relay for piston motion UP
}

// moves the drink closer to the IR (starting position) with incremental steps
// -> return true on successful motion (returns false otherwise)
bool resetRail()
{

  pistonDown(); 

  int QRE_Value = 0;
  while(QRE_Value < IR_THRESHOLD) {
    myMotor->step(1, FORWARD, DOUBLE);  // move closer

    delay(2); // give the motor time to move
    QRE_Value = digitalRead(QRE1113_Pin);
  }

  // reached start
  pistonUp();
  return true;

}

// fuck this is an open loop system with one sensor at start - this is the best i can do :/
// The stepper is also counting steps
bool isStepperAtStart()
{
  if (digitalRead(QRE1113_Pin) > IR_THRESHOLD) {
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
      msg_buffer[i] = (int)(message[i]);
    }
  }

}

// Takes the user input and builds a path for the servo as a list of steps with directions
// returns a list of instructions for the stepper (2d vector) [direction, steps, shots]
// direction defined as 0 - FORWARD (closer to start), 1 - BACKWARD (away from start) 
void pathPlanner(int msg_buffer[], int path_buffer[][2])
{
  // Note this function can be later optimized for better path planning
  
  while (!isStepperAtStart()) { resetRail(); }

  int instruction [2] = {0,0};
  for (int d = 0; d < sizeof(msg_buffer); d++) {

    if (msg_buffer[d] != 0) {
      instruction[0] = bottle_position[d];
      instruction[1] = msg_buffer[d];

      *path_buffer[d] = instruction;
    }
  }

}

// Execution of path - moves cup through positions and pours liquor
// Assumption: only requested liquors are in the path buffer
void exec(int path_buf[][2])
{

  resetRail();

  // since resetRail... therefore... 
  int loc = 0;
  uint8_t dir;

  for (int i = 0; i < sizeof(path_buf); i++) {

    // move stepper to position until within margin of error - 10 steps
    while (abs(path_buf[i][0] - loc) < 10) {
      if (path_buf[i][0] > loc) {
        myMotor->step(1, BACKWARD, DOUBLE);
      } else {
        myMotor->step(1, FORWARD, DOUBLE);
      }

      delay(2); // give the motor time to move
    }

    for (int j = 0; j < path_buf[i][1]; j++) {

      delay(50);
      pistonUp();
      delay(DRINK_HOLD_TIME);
      pistonDown();
      delay(50);

    }

    
    delay(300);

  }

  resetRail();
}

void setup() 
{ 
  Serial.begin(9600); // init serial - no direct imports

  while (!Serial)
  {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  while ( !(init_sys()) ) {}  // run init until it works and system is ready to go (return true)
  
} 

void loop() 
{

  if (Serial.available()) { // keep checking serial
    userInput(pmsg);
    
    // path planner
    pathPlanner(pmsg, path);
    
    // exec (move servo and actuate piston)
    exec(path);
    
    // return home

  }

  // wait for new input

}

