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

const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns
//define the cymbols on the buttons of the keypads
char hexaKeys[ROWS][COLS] = {
  {'1','2','3', 'A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {19, 21,22, 23}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {2, 4, 5, 18}; //connect to the column pinouts of the keypad

//initialize an instance of class NewKeypad
Keypad customKeypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 




void setup(){
  Serial.begin(9600);
}
String inputString = "";
int ID_len = 0;
void loop(){
  char customKey = customKeypad.getKey();

    if(customKey){
      if(customKey == 'A' || customKey == 'B' || customKey == 'C' || customKey == 'D'){
         Serial.println("illegal value");
      }
      else if(customKey == '*'){
          if(ID_len == 0) {
            Serial.println("Nothing to undo");
          } else {
            inputString = inputString.substring(0,inputString.length() - 1);
            ID_len -= 1;
            Serial.println(inputString);
          }
      }
      else if(customKey == '#'){
          if(ID_len != 9) {
            Serial.println("invalid ID length");
          } else {
            Serial.println("ID sent to approval queue");
          }
          inputString = "";
          ID_len = 0;
      }
      else{
        if(ID_len == 9) {
          Serial.println("Maximum length excceded");
        } else {
          inputString += customKey;
          ID_len += 1;
          Serial.println(inputString);
        }
      }
    }
}