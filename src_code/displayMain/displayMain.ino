/* 
LCD Display funcitionality
Author: Guy Meyer + Contribution from online "create.arduino.cc"
Date: Oct 2 2017
*/ 

//Sample using LiquidCrystal library
#include <LiquidCrystal.h>
 
// select the pins used on the LCD panel
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
 
// define some values used by the panel and buttons
int lcd_key     = 0;
int adc_key_in  = 0;
#define btnRIGHT  0
#define btnUP     1
#define btnDOWN   2
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5

// Drink Options
char d1[] = "Vodka Cran";
char d2[] = "Gin & Tonic";
char d3[] = "Rum & Coke";
char* drink_names[] = {d1,d2,d3};
int drinks[] = {1,2,3};  // corressponding to Drink Options (above)
int current_drink = 0;
int num_drinks = sizeof(drinks)/2;

// Variables for menu function:
int buttonPushCounter = 0;   // counter for the number of button presses
int buttonState = 0;         // current state of the button
int lastButtonState = 0;     // previous state of the button


// read the buttons
int read_LCD_buttons() {
  adc_key_in = analogRead(0);      // read the value from the sensor
  // my buttons when read are centered at these valies: 0, 144, 329, 504, 741
  // we add approx 50 to those values and check to see if we are close
  if (adc_key_in > 1000) return btnNONE; // We make this the 1st option for speed reasons since it will be the most likely result
  if (adc_key_in < 50)   return btnRIGHT; 
  if (adc_key_in < 195)  return btnUP;
  if (adc_key_in < 380)  return btnDOWN;
  if (adc_key_in < 555)  return btnLEFT;
  if (adc_key_in < 790)  return btnSELECT;  
  return btnNONE;  // when all others fail, return this...
}

// menu options 
int scroll_menu(int pressed, int current) {

  buttonState = pressed;
  
  // compare the buttonState to its previous state
  if (buttonState != lastButtonState) {
    
    if (buttonState == 2 && current != num_drinks - 1){
      current = (current + 1) % num_drinks;        // scroll down
    } else if (buttonState == 1 && current != 0) {
      current = (current - 1) % num_drinks;        // scroll up
    } else if (buttonState == 4) {
      makeDrink(current);
      delay(50);
    }
    
  } else {
    current = current;
  }
  // Delay a little bit to avoid bouncing
  delay(50);
  
  // save the current state as the last state, for next time through the loop
  lastButtonState = buttonState;
  
  return current;

} 


void clearLine(int line) {
  lcd.setCursor(0,line);
  lcd.print("               ");
  lcd.setCursor(0,line);
}


void makeDrink(int drink_num) {
  clearLine(0);

  int delay_time = 500;
  
  Serial.write(drink_num);

  while(!(Serial.available())){
    lcd.setCursor(0,0);
    lcd.print("Preparing.  ");
    delay(delay_time);
    lcd.setCursor(0,0);
    lcd.print("Preparing.. ");
    delay(delay_time);
    lcd.setCursor(0,0);
    lcd.print("Preparing...");
    delay(delay_time);
  }
  
  Serial.read();
  
  clearLine(0);
  lcd.print("Enjoy your");
  
  lcd.setCursor(13,1);
  lcd.print(" :)");
  
  exit(-1);
  
}


void setup()
{
  Serial.begin(9600);  // initiates serial communication
  
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  
  lcd.begin(16, 2);              // start the library
  lcd.setCursor(0,0);
  lcd.print("Select Drink..."); // print a simple message
}
  
void loop()
{
  /*
  lcd.setCursor(9,1);            // move cursor to second line "1" and 9 spaces over
  lcd.print(millis()/1000);      // display seconds elapsed since power-up
 */
 
  lcd.setCursor(0,1);            // move to the begining of the second line
  lcd_key = read_LCD_buttons();  // read the buttons
 
  // may terminate while program during function call if drink selected
  current_drink = scroll_menu(lcd_key,current_drink);
  
  // clear bottom line
  clearLine(1);
  
  // displays drink in list
  lcd.print(drink_names[current_drink]);
  lcd.setCursor(13,1);
  lcd.print(current_drink + 1);
  lcd.print("/");
  lcd.print(num_drinks);
 
}




