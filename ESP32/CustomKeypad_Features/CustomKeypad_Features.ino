/* @file CustomKeypad.pde
|| @version 1.0
|| @author Alexander Brevig
|| @contact alexanderbrevig@gmail.com
||
|| @description
|| | Demonstrates changing the keypad size and key values.
|| #
*/
#include <Keypad.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns
//define the cymbols on the buttons of the keypads
char hexaKeys[ROWS][COLS] = {
  {'1','2','3', 'A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {19, 27,26, 25}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {2, 4, 5, 18}; //connect to the column pinouts of the keypad

//initialize an instance of class NewKeypad
Keypad customKeypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);


void setup(){
  Serial.begin(9600);
  

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  delay(1000);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  // Display static text
  display.println("Begin");
  display.display(); 

}

String inputString = "";
int ID_len = 0;
void loop(){
  char customKey = customKeypad.getKey();
    if(customKey){
      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(0, 10);
      display.println("Please enter ID:");
      display.display(); 
      if(customKey == 'A' || customKey == 'B' || customKey == 'C' || customKey == 'D'){
          display.setCursor(0, 50);
          display.println("Illegal Value X");
          display.display(); 
          Serial.println("illegal value");
      }
      else if(customKey == '*'){
          if(ID_len == 0) {
            display.setCursor(0, 50);
            display.println("Nothing to undo");
            display.display(); 
            Serial.println("Nothing to undo");
          } else {
            inputString = inputString.substring(0,inputString.length() - 1);
            ID_len -= 1;
            display.setCursor(0, 25);
            display.println(inputString);
            display.display(); 
            display.setCursor(0, 50);
            display.println("Undo");
            display.display(); 
            Serial.println(inputString);
          }
      }
      else if(customKey == '#'){
            display.clearDisplay();
            display.setTextSize(1);
            display.setTextColor(WHITE);
            display.setCursor(0, 10);
          if(ID_len != 9) {
            display.println("Invalid ID length");
            display.setCursor(0, 25);
            display.println("Try again!");
            display.display(); 
            Serial.println("invalid ID length");
          } else {
            display.println("Success!");
            display.setCursor(0, 25);
            display.println("ID pending approval");
            display.display(); 
            Serial.println("ID sent to approval queue");
          }
          inputString = "";
          ID_len = 0;
      }
      else{
        if(ID_len == 9) {
          display.setCursor(0, 50);
          display.println("Maximum length!");
          display.display(); 
          Serial.println("Maximum length excceded");
        } else {
          inputString += customKey;
          ID_len += 1;
          
          Serial.println(inputString);
        }
        display.setCursor(0, 25);
        display.println(inputString);
        display.display(); 
      }
    }
}
