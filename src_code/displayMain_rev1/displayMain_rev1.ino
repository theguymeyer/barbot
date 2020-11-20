/* 
LCD Display funcitionality
Author: Guy Meyer + Contribution from online "create.arduino.cc"
Last Edited: Mar 10 2020

Display Controller responsible for a couple of functions:
  1) Power and control an LCD display that allows the user to interact in realtime
  2) List drink options on LCD Display
  3) Allow user to select desired drink
  4) Upon user request, send drink data to main controller which handles the execution
  5) Notify user that their drink is complete
  
Bottle Info:
  Bottle 1 - Vodka (750 ml)
  Bottle 2 - Gin (750 ml)
  Bottle 3 - Rum 
  Bottle 4 - Clear wine bottle filled with Coca-Cola (labels removed)
  Bottle 5 - Clear wine bottle filled with Tonic Water (labels removed)
  
Arduino Pinouts:
D0 - Serial Rx
D1 - Serial Tx
D2 - 
D3 - 
D4 - Used by LCD
D5 - Used by LCD
D6 - Used by LCD
D7 - Used by LCD
D8 - Used by LCD
D9 - Used by LCD
D10 - 
D11 - 
D12 - 
D13 - 
A1 - 
A2 - 
A3 - 
A4 - 
A5 - 
5V - 
GND - 
Vin - 

*/ 

//Sample using LiquidCrystal library
#include <LiquidCrystal.h>
 
// select the pins used on the LCD panel
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
 
// define some values used by the panel and buttons
// TODO these button options should be an enumerated variable
#define btnRIGHT  0
#define btnUP     1
#define btnDOWN   2
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5
int lcd_key     = 0;
int adc_key_in  = 0;

// Drink Options
char d1[] = "Vodka & Cran";
char d2[] = "Whisky Coke";
char d3[] = "Red Wine";
char* drink_names[] = {d1,d2,d3};
char drinks_serial_ID[] = {'0','1','2'};  // corresponding to Drink Options (above)
int current_drink = 0;  // integer referencing the position in the list of available drinks
int total_drinks = sizeof(drinks_serial_ID);

// Variables for menu function:
int buttonPushCounter = 0;   // counter for the number of button presses
int buttonState = 0;         // current state of the button - TODO make enum?
int lastButtonState = 0;     // previous state of the button - TODO make enum?

/* LCD Functions */

void clearLine(int line) {
  lcd.setCursor(0,line);
  lcd.print("               ");
  lcd.setCursor(0,line);
}

  
void lcdUpdateDrink(int drink_num)
{
  clearLine(1);
  
  // displays drink in list
  lcd.setCursor(0,1);            // move to the begining of the second line
  lcd.print(drink_names[drink_num]);
  
  // displays list position
  lcd.setCursor(13,1);
  lcd.print(drink_num + 1);
  lcd.print("/");
  lcd.print(total_drinks);
}

// read the buttons
int read_LCD_buttons() {
  adc_key_in = analogRead(0);      // read the value from the sensor
  // my buttons when read are centered at these valies: 0, 144, 329, 504, 741
  // we add approx 50 to those values and check to see if we are close
  // TODO should be a switch case
  if (adc_key_in > 1000) return btnNONE; // We make this the 1st option for speed reasons since it will be the most likely result
  if (adc_key_in < 50)   return btnRIGHT; 
  if (adc_key_in < 195)  return btnUP;
  if (adc_key_in < 380)  return btnDOWN;
  if (adc_key_in < 555)  return btnLEFT;
  if (adc_key_in < 790)  return btnSELECT;  
  return btnNONE;  // when all others fail, return this...
}

/* User Functions / Custom to Functionality */

// menu options 
int scroll_menu(int button_pressed, int current) {
  
  /*
  Serial.print(button_pressed);
  Serial.print(",");
  Serial.println(current);  
  */
  
  // compare the buttonState to its previous state
  if (button_pressed != lastButtonState) {
    
    // track button pressed state for next iteration
    lastButtonState = button_pressed;
    
    if (button_pressed == 2 && current < (total_drinks - 1)){ // scroll down
      
      // displays drink in list
      lcdUpdateDrink(current + 1);
      
      return (current + 1);

    } else if (button_pressed == 1 && current != 0) { // scroll up
      
      // displays drink in list
      lcdUpdateDrink(current - 1);
      
      return (current - 1);

    } else if (button_pressed == 4) {  // select button
      make_drink(current);
      delay(6000);
      
      // prepare for new drink
      initialize_lcd();
    }
    
  }
  
  
  return current;

} 

void make_drink(int drink_num) {
  clearLine(0);

  int delay_time = 500;
  
  Serial.write(drinks_serial_ID[drink_num]);

  lcd.setCursor(0,0);
  lcd.print("Preparing");
  while(!(Serial.available())){
    // load screen
    lcd.setCursor(9,0);
    lcd.print(".  ");
    delay(delay_time);
    lcd.setCursor(9,0);
    lcd.print(".. ");
    delay(delay_time);
    lcd.setCursor(9,0);
    lcd.print("...");
    delay(delay_time);
  }
  
  Serial.read();
  
  clearLine(0);
  lcd.print("Enjoy your");
  
  lcd.setCursor(13,1);
  lcd.print(" :)");
  
  return;
  
}

void initialize_lcd()
{
  lcd.setCursor(0,0);
  lcd.print("Select Drink..."); // print a simple message
  
  lcdUpdateDrink(0);
}


void setup()
{
  Serial.begin(9600);  // initiates serial communication
  
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  
  lcd.begin(16, 2);              // start the library
  
  initialize_lcd();
}
  
void loop()
{
  lcd.setCursor(0,1);            // move to the begining of the second line
  lcd_key = read_LCD_buttons();  // read the buttons
 
  // may terminate while program during function call if drink selected - ugh what?
  current_drink = scroll_menu(lcd_key, current_drink);
 
  // clear bottom line
  //clearLine(1);
  
  delay(100);
 
}
  





