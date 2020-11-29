// Author: Guy Meyer
// Date of creation: May 8th 2020
// Last change date: May 10th 2020
// Objective: to create an interactive method for drink selection.
// System Description:
//	- Hardware: There is no direct hardware interface in this module. The board is an Arduino Uno (ATMEGA238P) with a 16x2 LCD Display and 6 push buttons {UP,DOWN,LEFT,RIGHT,SELECT,RESET}
//
//	- Electrical: Arduino inputs 9VDC and outputs 5VDC to the LCD display. The RESET button is electrically linked to the power of the board and will perform a hard reset (ie. reinitialization).
//              The module is not concerned with the electrical limitaiton of the board
//
//	- Software: OUTPUT: mainFile.ino receives a serial message indicating the drink requirements based on 5 unique bottle
//			serial message structure: uint8{Bottle_1, Bottle_2, Bottle_3, Bottle_4, Bottle_5} @ BAUDRATE=9600 via RX/TX serial bus
//			where each Bottle is a byte describing the number of shots from each bottle in ASCII. The system is not concerned with speed optimization with the serial buffer
//
//			INPUT: user taps on any combination of the six keys.
//
// Design: the LCD display will show the shot amounts of all the bottles, and a label will indicate which bottles are requested. This allows for the system to be dynamic and allow different bottles.
//          the user will initialize on the first bottle which will be flashing to place the locus of the user on the first bottle.
//          The user has the option of performing 6 actions {to_next_bottle, to_prev_bottle, inc_current_bottle, dec_current_bottle, send_drink, reset(via hardware)}
//
//          the user will have no time restrictions for building their drink.
//
//          once the user selects send_drink the system will freeze and await a serial message back from mainFile.ino indicating the drink is complete.
//          the UI will display a "thank you" message and timeout with a reset after 5 seconds
//
//          upon reset a new drink can be built

// IMPLEMENTATION

// Imports

// LiquidCrystal library
#include <LiquidCrystal.h>
#include "src/drink.h"
#include "src/user.h"

// pins used by LCD display
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

// the drink
Drink drink;
char new_drink_struct[5] = "00000";

// the user
User user;

// Defined Variables

// define some values used by the panel and buttons - Author https://gist.github.com/pws5068/3279865, used for LCD interface
int lcd_key = 0;
int adc_key_in = 1050;  // Start with btnNONE state
#define btnRIGHT 0
#define btnUP 1
#define btnDOWN 2
#define btnLEFT 3
#define btnSELECT 4
#define btnNONE 5
#define btnBKSP 6

// useful definitions
#define clearLCD "               "
#define debounceTime 100 //milliseconds

// debounce Variables
long debounceReleaseTime = 0;

// ---- Useful Functions for LCD ----

// read the buttons - Author https://gist.github.com/pws5068/3279865, used for LCD interface
int read_LCD_buttons()
{

  if (analogRead(A1) < 30) { return btnBKSP; }  // reset screen
  
  adc_key_in = analogRead(0); // read the value from the sensor
  // my buttons when read are centered at these valies: 0, 144, 329, 504, 741
  // we add approx 50 to those values and check to see if we are close
  if (adc_key_in > 1000)
    return btnNONE; // We make this the 1st option for speed reasons since it will be the most likely result
  if (adc_key_in < 50)
    return btnRIGHT;
  if (adc_key_in < 195)
    return btnUP;
  if (adc_key_in < 380)
    return btnDOWN;
  if (adc_key_in < 555)
    return btnLEFT;
  if (adc_key_in < 790)
    return btnSELECT;
  return btnNONE; // when all others fail, return this...
}

// LCD functions

void writeToLCD(char *drink_structure)
{
  char top_string[16] = "";
  char bottom_string[16] = "";

  char top_char = ' ';
  char bottom_char = ' ';

  int count = (int)bottleCount;

  for (int i = 0; i < count; i++)
  {
    if (drink_structure[i] == '0')
    {
      strcat(top_string, "   ");
      strcat(bottom_string, "   ");
    }
    else if (drink_structure[i] == '1')
    {
      strcat(top_string, "   ");
      strcat(bottom_string, "## ");
    }
    else if (drink_structure[i] == '2')
    {
      strcat(top_string, "## ");
      strcat(bottom_string, "## ");
    }
  }

  // print strings
  lcd.setCursor(0, 0);
  lcd.print(clearLCD);
  lcd.setCursor(0, 0);
  lcd.print(top_string);

  lcd.setCursor(0, 1);
  lcd.print(clearLCD);
  lcd.setCursor(0, 1);
  lcd.print(bottom_string);
}

void userLocationLCD(User *this_user, Drink *this_drink)
{
  int user_pos = this_user->get_user_position();

  lcd.setCursor(user_pos * 3, 1);
  lcd.print("^");
}

// debounce function
int timeToDebounce(int lastButtonPress)
{
  return lastButtonPress + (int)debounceTime;
}

// clear lcd
void clearDisplay()
{
  lcd.setCursor(0, 0);
  lcd.print(clearLCD);
  lcd.setCursor(0, 1);
  lcd.print(clearLCD);

  lcd.setCursor(0, 0);
}

// wait on status update from mainFile
void wait_update()
{

  // give time for serial msg to send
  delay(1000);

  clearDisplay();
  lcd.print("Working...");

  delay(20);

  while (!Serial.available()) {}

  // prepare to write msg
  lcd.setCursor(0, 0);
  lcd.print(clearLCD);
  lcd.setCursor(0, 1);
  lcd.print(clearLCD);

  lcd.setCursor(0, 0);

  String message = Serial.readString();

  if (message == "DONE") {
    lcd.print("Enjoy :)");
  } else {
    lcd.print("Error 001");

  }

  delay(3000);

  // clean up
  Serial.flush();
  Serial.flush();


}

void systemReset()
{
  user.init_user();
  drink.init_drink(&new_drink_struct[0]);

  clearDisplay();
}

// Arduino Stateflow (the 'main') - includes setup() and loop()
void setup()
{
  Serial.begin(9600); // initiates serial communication

  while (!Serial)
  {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  Serial.print("OK\n");

  lcd.begin(16, 2); // start the library
  lcd.setCursor(0, 0);
  pinMode(A1, INPUT_PULLUP);

  user.init_user();
  drink.init_drink(&new_drink_struct[0]);
}

void loop()
{
  writeToLCD(drink.get_drink());

  userLocationLCD(&user, &drink);
  
  if (millis() > debounceReleaseTime)
  {

    lcd_key = read_LCD_buttons(); // read the buttons

    switch (lcd_key) // depending on which button was pushed, we perform an action
    {
    case btnRIGHT:
    {
      user.inc_bottle_position();
      break;
    }
    case btnLEFT:
    {
      user.dec_bottle_position();
      break;
    }
    case btnUP:
    {
      // increase current drink
      drink.update_drink_by_bottle(user.get_user_position(), true);
      break;
    }
    case btnDOWN:
    {
      //decrease current drink
      drink.update_drink_by_bottle(user.get_user_position(), false);
      break;
    }
    case btnSELECT:
    {
      Serial.print(drink.get_drink());
      wait_update();
      systemReset();
      break;
    }
    case btnBKSP:
    {
      // reset user & reset drink
      systemReset();
      break;
    }
    case btnNONE:
    {
      break;
    }
    }

    // set new button release time
    debounceReleaseTime = millis() + (long)debounceTime;
  }

  delay(50);
}
