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

void setup() 
{ 
  
  //Serial.begin(9600);  // initiates serial communication
  
} 

void loop() 
{

}

