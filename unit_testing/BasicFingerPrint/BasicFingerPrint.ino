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
#include <TimeLib.h> 
#include "SPIFFS.h"
#include <ArduinoJson.h>


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
const int redPin = 13;
const int greenPin = 12;
const int bluePin = 14;

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
String inputString = "";
bool id_arr[162] = {false}; //fingerprint id 
uint8_t id;
const int MAX_USERS = 162;
time_t current_time;
uint8_t fingerPrintID;
String id_str;
const char* path = "/data_users.json";

void fingerPrintSensorClear();
void initiatefile();


void setup() {
  Serial.begin(115200);
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  
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

  //----------- initiate SPIFFS -------------
  bool success = SPIFFS.begin();
  SPIFFS.format();
  
  if (success) {
    Serial.println("File system mounted with success");
  } else {
    Serial.println("Error mounting the file system");
    return;
  }
  
  /*initiatefile();*/
  /*
  if (SPIFFS.format()) {
    Serial.println("SPIFFS formatted successfully");
  } else {
    Serial.println("Failed to format SPIFFS");
  }*/
  
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

  //fingerPrintSensorClear(); // very importantttt 
  finger.getTemplateCount();
  Serial.print("Sensor contains "); Serial.print(finger.templateCount); Serial.println(" templates");
  Serial.println("Waiting for valid finger...");


  Serial.println("Files stored in SPIFFS:");
  File root = SPIFFS.open("/");
  File file = root.openNextFile();
  while (file) {
    Serial.println(file.name());
    file = root.openNextFile();
  }
}

void fingerPrintSensorClear() {
  for (int i = 1; i <= 162; ++i) {
    if (finger.deleteModel(i) == FINGERPRINT_OK) {
      Serial.println("Template deleted: " + String(i));
    } else {
      Serial.println("Failed to delete template: " + String(i));
    }
    delay(100); // Add a small delay between deletions
  }
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

/*
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
*/

String GetIdFromKeypad() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  // Display static text
  display.println("Please enter ID:");
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
  return "";
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
      display.print(F("Registering"));
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
  return p;
}


void setColor(int redValue, int greenValue, int blueValue) {
  analogWrite(redPin, redValue);
  analogWrite(greenPin, greenValue);
  analogWrite(bluePin, blueValue);
}

//////////////////////////////////////////////////////////////////////////////////
void saveUserDataToFile(int fingerprint_id, String id, bool is_appending, bool is_approved, time_t curr_time) {
    File file = SPIFFS.open(path, "a");
    if (!file) {
        Serial.println("Failed to open file for writing");
        return;
    }

    Serial.println("saveUserDataToFile---------- File opened successfully");

    DynamicJsonDocument doc(1024);

    JsonObject userObj = doc.createNestedObject();
    userObj["fingerprintID"] = fingerprint_id;
    userObj["id"] = id;
    userObj["isApproved"] = is_approved;
    userObj["isAppending"] = is_appending;
    //userObj["time"] = curr_time;

    if (serializeJson(doc, file)) {
        Serial.println("saveUserDataToFile----------- Data written to file successfully");
    } else {
        Serial.println("saveUserDataToFile------------ Failed to write data to file");
    }

    file.close();

    file = SPIFFS.open(path, "r");
    if(!file){
    Serial.println("Failed to open file for reading");
    return;
    }
   Serial.println("\n");
   Serial.println("File Content:");
    while(file.available()){
      Serial.write(file.read());
      Serial.println("\n");
  }
  Serial.println("\n");

  file.close();
}

// set approved true, isPennding to false, and update the time
void updateUserApprovedON(int targetFingerprint) {
    DynamicJsonDocument doc(1024);
    File file = SPIFFS.open(path, "r");
    if (!file) {
        Serial.println("Failed to open file for reading");
        return;
    }

    DeserializationError error = deserializeJson(doc, file);
    file.close();

    if (error) {
        Serial.println("Failed to read JSON");
        return;
    }

    for (JsonObject userObj : doc.as<JsonArray>()) {
        int fingerprint = userObj["fingerprintID"];
        if (fingerprint == targetFingerprint) {
            userObj["isApproved"] = true;
            userObj["isAppending"] = false;
            userObj["time"] = now(); 
            break; // Stop iterating once we find the target user
        }
    }

    File outFile = SPIFFS.open(path, "w");
    if (!outFile) {
        Serial.println("Failed to open file for writing");
        return;
    }

    serializeJson(doc, outFile);
    outFile.close();

    Serial.println("Updated isApproved for user with target fingerprint.");
}


bool isApprovedUser(int targetFingerprint) {
  DynamicJsonDocument doc(1024);
  File file = SPIFFS.open(path, "r");
  if (!file) {
    Serial.println("isApprovedUser --- Failed to open file for reading");
    return false;
  }

  DeserializationError error = deserializeJson(doc, file);
  file.close();

  if (error) {
    Serial.println("heree");
    Serial.println("Failed to read JSON");
    return false;
  }

  for (JsonObject userObj : doc.as<JsonArray>()) {
    int fingerprint = userObj["fingerprintID"];
    if (fingerprint == targetFingerprint) {
      bool isApproved = userObj["isApproved"];
      Serial.println("is appprooooveeeeedddddddd>>>>");
      Serial.println(isApproved);
      return isApproved;
    }
  }

  return false; // User not found
}


void updatingTheTimeForApprovedUser(int targetFingerprint) {
  DynamicJsonDocument doc(1024);
  File file = SPIFFS.open(path, "r");
  if (!file) {
    Serial.println("Failed to open file for reading");
    return;
  }

  DeserializationError error = deserializeJson(doc, file);
  file.close();

  if (error) {
    Serial.println("Failed to read JSON");
    return;
  }

  for (JsonObject userObj : doc.as<JsonArray>()) {
    int fingerprint = userObj["fingerprintID"];
    if (fingerprint == targetFingerprint) {
      userObj["timestamp"] = now(); // Update the time
      break; // Stop iterating once we find the target user
    }
  }

    File outFile = SPIFFS.open(path, "w");
    if (!outFile) {
        Serial.println("Failed to open file for writing");
        return;
    }

    serializeJson(doc, outFile);
    outFile.close();

    Serial.println("Updated time for user with target fingerprint.");
}

void printJsonFileContent(const char* filename) {
  Serial.println("print content: ");
    File file = SPIFFS.open(path, "r");
    if(!file){
    Serial.println("Failed to open file for reading");
    return;
    }
  
  Serial.println("File Content:");
  while(file.available()){
    Serial.write(file.read());
  }
  file.close();
}

void initiatefile(){
DynamicJsonDocument doc(10000);
  
  // Populate the data document with {key: value} pairs
  for (int i = 1; i <= 162; ++i) {
    JsonObject entry = doc.createNestedObject();
    entry["key"] = i;
    entry["value"] = false;
  }

  File jsonFile = SPIFFS.open("/fingerprintID.json", "w");
  if (!jsonFile) {
    Serial.println("Failed to open fingerprintID.json for writing");
    return;
  }

  serializeJsonPretty(doc, jsonFile); // Use serializeJsonPretty for indentation
  jsonFile.close();

  Serial.println("JSON data has been written to 'fingerprintID.json'");
}


int getIDForFingerPrint() {
  for (int current = 0; current <= 162; current++){
    if (id_arr[current] == false){
      return current+1;
    }
  }
  /*
  File dataFile = SPIFFS.open("/fingerprintID.json", "r+");
  if (!dataFile) {
    Serial.println("Failed to open data.json");
    return -1;
  }
  Serial.println("File Content:");
  while(dataFile.available()){
    Serial.write(dataFile.read());
  }
  size_t size = dataFile.size();
  std::unique_ptr<char[]> buf(new char[size]);

  dataFile.readBytes(buf.get(), size);
  dataFile.close();
 
  DynamicJsonDocument doc(10000);
  DeserializationError error = deserializeJson(doc, buf.get());
  if (error) {
    Serial.println("Failed to parse JSON");
    Serial.println(error.c_str());
    return -1;
  }

  for (JsonVariant entry : doc.as<JsonArray>()) {
    int key = entry["key"];
    bool value = entry["value"];

    if (!value) {
      Serial.print("First key with false value: ");
      Serial.println(key);
      entry["value"] = true;
      return key;
      break;
    }
  }
  return -1;
  */
  return -1;
}

void loop() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  // Display static text
  display.println("For Access, place");
  display.setCursor(0, 30);
  display.println("finger on sensor");
  display.display();
  setColor(0, 0, 255);
  ////////////////////////
  FingerID = getFingerprintID(); // Get the Fingerprint ID from the Scanner

  delay(50); 
  
  //User current_user;
  //Didn't find a match
  if (FingerID == -1) {
    setColor(255, 0, 0);
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 10);
    display.println("Access Denied! You");
    display.setCursor(0, 30);
    display.println("are not registered.");
    display.display();
  
    delay(3000);
  
    id = getIDForFingerPrint();
    id_arr[id - 1] = true;

    fingerPrintID = getFingerprintEnroll();
    id_str = GetIdFromKeypad();
    current_time = now();

    if (id > 0 && id <= 162) {
      saveUserDataToFile(id, id_str, true, false, current_time);
    }
    else {
      Serial.println("there is no avalibe place in the system\n");
    }
  } else if (FingerID > 0)  {
    if (isApprovedUser(FingerID)) {
      updatingTheTimeForApprovedUser(fingerPrintID);

      setColor(0, 255, 0);
      delay(1000);
      display.println("Welcome!\n");

    } else {
      setColor(255, 0, 255);
      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(0, 10);
      // Display static text
      display.println("Access Denied, you");
      display.setCursor(0, 30);
      display.println("are in the waiting   list");
      display.display();
      delay(1000);
      Serial.println("\nYou are in the pending list mannn!, wait until the ceo accept your request\n");
      printJsonFileContent(path);
    }
  }
  display.display();
  delay(1000); // Delay to prevent rapid fingerprint reading

  
}
