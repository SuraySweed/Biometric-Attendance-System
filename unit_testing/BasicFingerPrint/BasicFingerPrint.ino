#include <SPI.h>
#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Fingerprint.h>
#include "helper.h"
#include "credentialsData.h"
#include "User.h"
#include <vector>
#include <algorithm>
#include <Keypad.h>

// Pins connected to the fingerprint sensor
#define FINGERPRINT_RX_PIN 16
#define FINGERPRINT_TX_PIN 17

HardwareSerial fingerSerial(1);  // Use hardware serial port 1
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&fingerSerial);

// Declaration for SSD1306 display connected using software I2C
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET 0 // Reset pin # (or -1 if sharing Arduino reset pin)

using std::vector;

const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns

char hexaKeys[ROWS][COLS] = {
  {'1','2','3', 'A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};


byte rowPins[ROWS] = {19, 21,22, 23}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {2, 4, 5, 18}; //connect to the column pinouts of the keypad

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Keypad customKeypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 

int FingerID; // The Fingerprint ID from the scanner
vector<User> pending_users;
vector<User> approved_users;
String inputString = "";
bool id_arr[161] = {false}; //fingerprint id 
uint8_t id;


typedef struct firebaseUserDetails_t {
  uint8_t fingerprint_id;
  bool is_approval;
} FB_User;

void setup() {
  Serial.begin(115200);

  //-----------initiate OLED display-------------
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  // you can delet these three lines if you don't want to get the Adfruit logo appear
  display.display();
  delay(2000); // Pause for 2 seconds
  display.clearDisplay();
  
  // set the data rate for the sensor serial port
  fingerSerial.begin(57600, SERIAL_8N1, FINGERPRINT_RX_PIN, FINGERPRINT_TX_PIN);
  Serial.println("\n\nAdafruit finger detect test");

  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
    display.clearDisplay();
    display.drawBitmap( 34, 0, FinPr_valid_bits, FinPr_valid_width, FinPr_valid_height, WHITE);
    display.display();
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    display.clearDisplay();
    display.drawBitmap( 32, 0, FinPr_failed_bits, FinPr_failed_width, FinPr_failed_height, WHITE);
    display.display();
    while (1) { delay(1); }
  }
  
  finger.getTemplateCount();
  Serial.print("Sensor contains "); Serial.print(finger.templateCount); Serial.println(" templates");
  Serial.println("Waiting for valid finger...");
}

int getIDForFingerPrint() {
  for (int current = 0; current < 161; current++) {
    if (id_arr[current] == false) {
      return current + 1;
    }
   }
   return -1;
}

int getFingerprintID() {
  uint8_t p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK:
    //Serial.println("Image taken");
    break;
    case FINGERPRINT_NOFINGER:
    //Serial.println("No finger detected");
    return 0;
    case FINGERPRINT_PACKETRECIEVEERR:
    //Serial.println("Communication error");
    return -2;
    case FINGERPRINT_IMAGEFAIL:
    //Serial.println("Imaging error");
    return -2;
    default:
    //Serial.println("Unknown error");
    return -2;
  }
  // OK success!
  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
    //Serial.println("Image converted");
    break;
    case FINGERPRINT_IMAGEMESS:
    //Serial.println("Image too messy");
    return -1;
    case FINGERPRINT_PACKETRECIEVEERR:
    //Serial.println("Communication error");
    return -2;
    case FINGERPRINT_FEATUREFAIL:
    //Serial.println("Could not find fingerprint features");
    return -2;
    case FINGERPRINT_INVALIDIMAGE:
    //Serial.println("Could not find fingerprint features");
    return -2;
    default:
    //Serial.println("Unknown error");
    return -2;
  }
  // OK converted!
  p = finger.fingerFastSearch();
  if (p == FINGERPRINT_OK) {
    //Serial.println("Found a print match!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    //Serial.println("Communication error");
    return -2;
  } else if (p == FINGERPRINT_NOTFOUND) {
    //Serial.println("Did not find a match");
    return -1;
  } else {
    //Serial.println("Unknown error");
    return -2;
  }
  // found a match!
  //Serial.print("Found ID #"); Serial.print(finger.fingerID);
  //Serial.print(" with confidence of "); Serial.println(finger.confidence);
  return finger.fingerID;
}


void DisplayFingerprintID(){
  //Fingerprint has been detected
  if (FingerID > 0) {
    display.clearDisplay();
    display.drawBitmap( 34, 0, FinPr_valid_bits, FinPr_valid_width, FinPr_valid_height, WHITE);
    display.display();
  }
  
  //No finger detected
  else if (FingerID == 0) {
    display.clearDisplay();
    display.drawBitmap( 32, 0, FinPr_start_bits, FinPr_start_width, FinPr_start_height, WHITE);
    display.display();
  }
  
  //Didn't find a match
  else if (FingerID == -1) {
    display.clearDisplay();
    display.drawBitmap( 34, 0, FinPr_invalid_bits, FinPr_invalid_width, FinPr_invalid_height, WHITE);
    display.display();
  }
  
  //Didn't find the scanner or there an error
  else if (FingerID == -2) {
    display.clearDisplay();
    display.drawBitmap( 32, 0, FinPr_failed_bits, FinPr_failed_width, FinPr_failed_height, WHITE);
    display.display();
  }
}

bool isApprovedUser(uint8_t targetId) {
  for (User user : approved_users) {
    if (user.getFingerprintID() == targetId && user.isUserApproved()) {
      return true;
    }
  }
  return false;
}

String GetIdFromKeypad() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  // Display static text
  display.println("Begin");
  display.display();


  String inputString = "";
  int ID_len = 0;
  
  while (ID_len <= 9) {
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
          return inputString;
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
}

uint8_t getFingerprintEnroll() {
  int p = -1;
  display.clearDisplay();
  display.drawBitmap( 34, 0, FinPr_scan_bits, FinPr_scan_width, FinPr_scan_height, WHITE);
  display.display();
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      //Serial.println("Image taken");
      display.clearDisplay();
      display.drawBitmap( 34, 0, FinPr_valid_bits, FinPr_valid_width, FinPr_valid_height, WHITE);
      display.display();
      break;
    case FINGERPRINT_NOFINGER:
      //Serial.println(".");
      display.setTextSize(1); // Normal 2:2 pixel scale
      display.setTextColor(WHITE); // Draw white text
      display.setCursor(0,0); // Start at top-left corner
      display.print(F("scanning"));
      display.display();
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      display.clearDisplay();
      display.drawBitmap( 34, 0, FinPr_invalid_bits, FinPr_invalid_width, FinPr_invalid_height, WHITE);
      display.display();
      break;
      case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }
  // OK success!
  p = finger.image2Tz(1);
  switch (p) {
  case FINGERPRINT_OK:
    display.clearDisplay();
    display.drawBitmap( 34, 0, FinPr_valid_bits, FinPr_valid_width, FinPr_valid_height, WHITE);
    display.display();
    break;
  case FINGERPRINT_IMAGEMESS:
    display.clearDisplay();
    display.drawBitmap( 34, 0, FinPr_invalid_bits, FinPr_invalid_width, FinPr_invalid_height, WHITE);
    display.display();
    return p;
  case FINGERPRINT_PACKETRECIEVEERR:
    Serial.println("Communication error");
    return p;
  case FINGERPRINT_FEATUREFAIL:
    Serial.println("Could not find fingerprint features");
    return p;
  case FINGERPRINT_INVALIDIMAGE:
    Serial.println("Could not find fingerprint features");
    return p;
  default:
    Serial.println("Unknown error");
    return p;
  }
  display.clearDisplay();
  display.setTextSize(2); // Normal 2:2 pixel scale
  display.setTextColor(WHITE); // Draw white text
  display.setCursor(0,0); // Start at top-left corner
  display.print(F("Remove"));
  display.setCursor(0,20);
  display.print(F("finger"));
  display.display();
  //Serial.println("Remove finger");
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  Serial.print("ID "); Serial.println(id);
  p = -1;
  display.clearDisplay();
  display.drawBitmap( 34, 0, FinPr_scan_bits, FinPr_scan_width, FinPr_scan_height, WHITE);
  display.display();
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      //Serial.println("Image taken");
      display.clearDisplay();
      display.drawBitmap( 34, 0, FinPr_valid_bits, FinPr_valid_width, FinPr_valid_height, WHITE);
      display.display();
      break;
    case FINGERPRINT_NOFINGER:
      //Serial.println(".");
      display.setTextSize(1); // Normal 2:2 pixel scale
      display.setTextColor(WHITE); // Draw white text
      display.setCursor(0,0); // Start at top-left corner
      display.print(F("scanning"));
      display.display();
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }
  // OK success!
  p = finger.image2Tz(2);
  switch (p) {
  case FINGERPRINT_OK:
    //Serial.println("Image converted");
    display.clearDisplay();
    display.drawBitmap( 34, 0, FinPr_valid_bits, FinPr_valid_width, FinPr_valid_height, WHITE);
    display.display();
    break;
  case FINGERPRINT_IMAGEMESS:
    Serial.println("Image too messy");
    return p;
  case FINGERPRINT_PACKETRECIEVEERR:
    Serial.println("Communication error");
    return p;
  case FINGERPRINT_FEATUREFAIL:
    Serial.println("Could not find fingerprint features");
    return p;
  case FINGERPRINT_INVALIDIMAGE:
    Serial.println("Could not find fingerprint features");
    return p;
  default:
    Serial.println("Unknown error");
    return p;
  }
  // OK converted!
  Serial.print("Creating model for #"); Serial.println(id);
  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    //Serial.println("Prints matched!");
    display.clearDisplay();
    display.drawBitmap( 34, 0, FinPr_valid_bits, FinPr_valid_width, FinPr_valid_height, WHITE);
    display.display();
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    Serial.println("Fingerprints did not match");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }
  Serial.print("ID "); Serial.println(id);
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    //Serial.println("Stored!");
    display.clearDisplay();
    display.drawBitmap( 34, 0, FinPr_valid_bits, FinPr_valid_width, FinPr_valid_height, WHITE);
    display.display();
    //confirmAdding(); to web
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not store in that location");
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }
}

// add fingerprint to memory and get the id from the user
void addFingerprintAndUserIDToList(User current_user) {
  uint8_t fingerPrintID = getFingerprintEnroll();
  String id_str = GetIdFromKeypad();
  current_user.setID(id_str.c_str());
  current_user.setFingerPrintID(fingerPrintID);
  current_user.setCurrentTime();
}

User* getUserFromPendingList(uint8_t fingerprint_id) {
  for (User user : pending_users) {
    if (user.getFingerprintID() == fingerprint_id) {
      return &user;
    }
  }
  return nullptr;
}

void waitingUsersApproval(vector<FB_User> fb_vector) {
  for (FB_User user : fb_vector) {
    User* current_user = getUserFromPendingList(user.fingerprint_id);
    if (current_user && user.is_approval) {
      approved_users.push_back(*current_user);
    }
    else {
      if (finger.deleteModel(user.fingerprint_id)) {
        id_arr[user.fingerprint_id] = false;
        Serial.println("Fingerprint deleted successfully");
      } else {
        Serial.println("Failed to delete fingerprint");
      }
    }
  }
}


bool isApprovedUser(int user_fingerprintID) {
  for (User& user : pending_users) {
    //Serial.println(user_fingerprintID);
    Serial.println(String(user.getID().c_str()));
    if (user.getFingerprintID() == user_fingerprintID && user.getID() == "211585666") {
      return true;
    }
  }
  return false;
  
  /*
  for (User& user : approved_users) {
    if (user.getFingerprintID() == user_fingerprintID) {
      return true;
    }
  }
  return false;*/
}

void updatingTheTimeForApprovedUser(int fingerprint_id) {
  for (User user : approved_users) {
    if (user.getFingerprintID() == fingerprint_id) {
      user.setCurrentTime();
    }
  }
}

void loop() {
  ////////////////////////
  FingerID = getFingerprintID(); // Get the Fingerprint ID from the Scanner
  //Serial.println(FingerID);
  //Serial.println("\n");
  delay(50);
  DisplayFingerprintID();

  User current_user;

  //Didn't find a match
  if (FingerID == -1) {
    id = getIDForFingerPrint();
    id_arr[id - 1] = true;
    addFingerprintAndUserIDToList(current_user);
    pending_users.push_back(current_user);    
  } else if (FingerID > 0)  {

    if (isApprovedUser(FingerID)) {
      updatingTheTimeForApprovedUser(FingerID);
      display.println("Welcome!\n");
    } else {
      Serial.println("You are in the pending list mannn!, wait until the ceo accept your request\n");
    }
  }
  display.display();
  delay(1000); // Delay to prevent rapid fingerprint reading

  for (User& user: pending_users) {
    Serial.print(user.getFingerprintID());
  }

}
