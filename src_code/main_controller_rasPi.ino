/* 
Component Controller
  - stepper motor (200steps), 12V1A Solenoid Valve
Author: Guy Meyer
Date: Jan 29 2020

Main Objective: Run Drink Routines as dispatched by the UI Controller
*/ 

#include <Stepper.h>

// output pins for each bottle control
enum Position { Home = 0, Rum = 1, Vodka = 2, WRum = 3, Coke = 4, Cran = 5, mojitoMix = 6 };

// stepper steps for each bottle
int distances[] = { 00, 1000, 2500, 4000, 5500, 7000, 8500 };

// corresponding pins to control bottle valves
int posPins[] = { 2,3,4,5,6,7,13 };

int dispensed;  // counts the number of liquids dispensed
int dispensingSpeed = 3000; // milliseconds per liquid ounce
int myDist; // tracks stepper position in steps thru enum variable

/* Motor Settings */
const int stepsPerRevolution = 200;
Stepper myStepper(stepsPerRevolution, 8, 9, 10, 11); // init the stepper, pins 8 through 11
int stepCount = 0;  // number of steps the motor has taken
int stepperSpeed = 100;  // speed in RPMs (chnage this for different speeds)

/* Sensor Settings */
int homeLimitSwitchPin = 12;


void setup() {
  // initialize the serial port:
  Serial.begin(9600);

  // gives warnings on unspecificed pins
  for (int thisPin = 0; thisPin < sizeof(posPins); thisPin++) {
    pinMode(posPins[thisPin], OUTPUT);
  }

  pinMode(homeLimitSwitchPin, INPUT);

  myStepper.setSpeed(stepperSpeed);

  init_state();

}

void loop() {
  // put your main code here, to run repeatedly:

  // USER input in switch case for all drink routines
  //Serial.flush();
  while(!Serial.available()) {}
  char drink_id = Serial.read();
  //Serial.write(drink_id);

  switch(drink_id) {
    case 'r': { rum_coke(); break; }
    case 'v': { vodka_cran(); break; }
    case 'm': { mojito(); break; } 
    default: { break; }
  }

}

/* Critical Controller Functions */

void init_state() {
  //move_to(distances[Position(Home)]);
  resetCup();
  dispensed = 0;
}

// moves cup to (Distance) pos
void move_to(Position pos) {
  //Serial.write("moving to position\n");
  
  int dist = distances[Position(pos)];
  
  myStepper.step(dist - myDist);  // this is a blocking function
  myDist = dist;
}

// keep stepping back till limit switch reads high
void resetCup() {
  //Serial.write("Resetting Cup\n");

  int counter = 0;
  
  while (digitalRead(homeLimitSwitchPin) != HIGH) {
    myStepper.step(-1);
    counter++;
  }
}

// controller for BOTTLE Bus
//  * input: number between 0 and 6 inclusive
//      - 0 is home
//      - 1->6 represent each bottle
//  * output: begin pour at position
void pour(Position pos, int shots) {
  //Serial.write("pouring shots\n");
  
  digitalWrite(posPins[Position(pos)], HIGH);
  delay(dispensingSpeed * shots);
  digitalWrite(posPins[Position(pos)], LOW);
  
  dispensed = dispensed + shots;
}

/* Drink Routins */

// Rum and Coke Routine
// Bottle1 (1oz) + Bottle4 (3oz)
void rum_coke() {
  blinkFor(1);
  move_to(Rum);
  pour(Rum, 1);
  move_to(Coke);
  pour(Coke, 3);
  
  Serial.flush();
  Serial.write('F');  // finished code
}

// Vodka Cran Routine - STRONG!!
// Bottle2 (2oz) + Bottle5 (2oz)
void vodka_cran() {
  blinkFor(2);
  move_to(Vodka);
  pour(Vodka, 2);
  move_to(Cran);
  pour(Cran, 2);
  
  Serial.flush();
  Serial.write('F');  // finished code
}

// Mojito
// Bottle3 (1oz) + Bottle6 (3oz)
void mojito() {
  blinkFor(3);
  move_to(WRum);
  pour(WRum, 1);
  move_to(mojitoMix);
  pour(mojitoMix, 3);

  Serial.flush();
  Serial.write('F');  // finished code
}

/* Testing Functions */
void blinkFor(int num) {
  delay(2000);
  for (int i = 0; i < num; i++) {
    digitalWrite(13, HIGH);
    delay(500);
    digitalWrite(13,LOW);
    delay(500);
  }
}
