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
#include <map>
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>
#include <Preferences.h>

//#include <Arduino.h>


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
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};


byte rowPins[ROWS] = { 19,27, 26, 25 }; //connect to the row pinouts of the keypad
byte colPins[COLS] = { 15, 4, 5, 18 }; //connect to the column pinouts of the keypad

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

Preferences preferences;

int FingerID; // The Fingerprint ID from the scanner
String inputString = "";
const int MAX_USERS = 162;
time_t current_time;
uint8_t fingerPrintID;
String id_str;
const char* approved_users_path = "/approved_users.json";
const char* pending_users_path = "/pending_users.json";
const char* rejected_users_path = "/rejected_users.json";
const char* users_logs_path = "/users_logs.json";
unsigned long dataMillis = 0;
unsigned long WifiStartTime = 0; //for wifi
unsigned long connectionTimeout = 10000; // 10 seconds
unsigned long reconnectTime = 0;
unsigned long printFileContentTime = 0;
unsigned long printFileContentTimeOut = 180000; // 3 min to reconnect // need to change
unsigned long reconnectionTimeout = 120000; // 2 min to reconnect // need to change
unsigned long UpdateTime = 0;
unsigned long UpdateTimeout = 120000;   //update firebase every 2 mins //need to change
unsigned long ID_Timeout = 12000;
int fingerprintID_StartTime = 0;
unsigned long fingerprintID_Timeout = 20000; // 20 seconds for fingerprint
uint8_t id = 1;
unsigned int current_sent_id_to_FB = 0;
unsigned int users_logs_counter = 1;
unsigned int users_logs_FB_counter = 1;
String updated_users_from_FB_to_files = ""; // 2 options: rejected file or approved file
int fingerprint_timeout = -20;
bool hasConnected = false;
/* 2. Define the API Key */
/* 3. Define the project ID */
#define API_KEY "AIzaSyCQOPaLIyCOJI71ASLu-DvJTlsbGsqRupA"
#define FIREBASE_PROJECT_ID "biometric-attendance-sys-1deca"

/* 4. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "biometproj@gmail.com"
#define USER_PASSWORD "12345678project"

void fingerPrintSensorClear();
void initiatefile();
void printSPIFFSfiles();
void initiatePerefernces();

// Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

String buildZeroString() {
    String result = "";
    for (int i = 0; i < MAX_USERS; i++) {
        result += '0';
    }
    return result;
}

String modifyCharacter(String input, int index, char newChar) {
    // Check if the index is within the bounds of the string
    if (index >= 0 && index < input.length()) {
        // Create a new string with the modified character
        input[index] = newChar;
    }
    return input;
}

String updateUpdatingUserFromFBToArray(int user_id) {
    String modifiedString = modifyCharacter(updated_users_from_FB_to_files, user_id - 1, '1');
    return modifiedString;
}

bool isUserUpdatingToFileFromFB(int user_id) {
    char charAtIndex = updated_users_from_FB_to_files.charAt(user_id - 1);
    if (charAtIndex == '1') {
        return true;
    }
    return false;
}


void connectToFireBase() {
    Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

    config.api_key = API_KEY;

    /* Assign the user sign in credentials */
    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;

    /* Assign the callback function for the long running token generation task */
    config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

    fbdo.setResponseSize(2048);

    Firebase.begin(&config, &auth);

    Firebase.reconnectWiFi(true);
}

void connectToWifi() {
    WiFi.begin(ssid, password);
    // 25 seconds to connect to wifi
    WifiStartTime = millis();

    while ((WiFi.status() != WL_CONNECTED) && (millis() - WifiStartTime < connectionTimeout || WifiStartTime == 0)) {
        //WifiStartTime = millis();
        delay(1000);

        display.clearDisplay();
        display.drawBitmap(34, 0, Wifi_start_bits, Wifi_start_width, Wifi_start_height, WHITE);
        display.display();

        Serial.println("Connecting to WiFi...");
    }

    if (WiFi.status() == WL_CONNECTED) {
        display.clearDisplay();

        display.drawBitmap(34, 0, Wifi_connected_bits, Wifi_connected_width, Wifi_connected_height, WHITE);
        display.display();

        Serial.println("Connecting to WiFi...");

        Serial.println();

        Serial.print("Connected with IP: ");
        Serial.println(WiFi.localIP());
        Serial.println();

        connectToFireBase();
        Serial.println("configure the time ");
        configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
        Serial.println("\n");
    hasConnected = true;
    }
}

void setup() {
    Serial.begin(115200);
    pinMode(redPin, OUTPUT);
    pinMode(greenPin, OUTPUT);
    pinMode(bluePin, OUTPUT);

    updated_users_from_FB_to_files = buildZeroString();

    //-----------initiate OLED display-------------
    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x64
        Serial.println(F("SSD1306 allocation failed"));
        for (;;); // Don't proceed, loop forever
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
    }
    else {
        Serial.println("Error mounting the file system");
        return;
    }
    // set the data rate for the sensor serial port
    fingerSerial.begin(57600, SERIAL_8N1, FINGERPRINT_RX_PIN, FINGERPRINT_TX_PIN);
    Serial.println("\n\nAdafruit finger detect test");

    if (finger.verifyPassword()) {
        Serial.println("Found fingerprint sensor!");
        display.clearDisplay();
        display.drawBitmap(34, 0, FinPr_valid_bits, FinPr_valid_width, FinPr_valid_height, WHITE);
        display.display();
    }
    else {
        Serial.println("Did not find fingerprint sensor :(");
        display.clearDisplay();
        display.drawBitmap(32, 0, FinPr_failed_bits, FinPr_failed_width, FinPr_failed_height, WHITE);
        display.display();
        while (1) {
            delay(1);
        }
    }

    fingerPrintSensorClear(); // very importantttt
    finger.getTemplateCount();
    Serial.print("Sensor contains "); Serial.print(finger.templateCount); Serial.println(" templates");

    initiatePerefernces();

    connectToWifi();
}

void printFileContent(const char* file_path) {
    File file = SPIFFS.open(file_path, "r");
    if (!file) {
        Serial.print("printFileContent----------- Failed to open file for reading");
        Serial.print(file_path);
        Serial.println();
        return;
    }
    Serial.println("\n");
    Serial.print(file_path);
    Serial.println(" File Content:");

    while (file.available()) {
        Serial.write(file.read());
    }

    Serial.println();

    file.close();
}

void initiatePerefernces() {
    preferences.begin("myApp", false);

    if (preferences.isKey("id")) {
        // The "id" key exists in preferences
        preferences.putInt("id", 1);
        id = preferences.getInt("id", 0);
        Serial.print("ID exists. Value: ");
        Serial.println(id);
    }
    else {
        // The "id" key does not exist in preferences, so initialize it with a default value
        int defaultId = 1; // Set your desired default value here
        preferences.putInt("id", defaultId);
        Serial.print("ID did not exist. Initialized with default value: ");
        Serial.println(defaultId);
    }

    if (preferences.isKey("cuurent_pending_user_sent")) {
        current_sent_id_to_FB = preferences.getInt("cuurent_pending_user_sent", 0);
        Serial.print("cuurent_pending_user_sent exists. Value: ");
        Serial.println(current_sent_id_to_FB);
    }
    else {
        int defaultValue = 0;
        preferences.putInt("id", defaultValue);
        Serial.print("cuurent_pending_user_sent did not exist. Initialized with default value: ");
        Serial.println(defaultValue);
    }

    if (preferences.isKey("updated_users_from_FB_to_files")) {
        updated_users_from_FB_to_files = preferences.getString("updated_users_from_FB_to_files", "");
    }
    else {
        preferences.putString("updated_users_from_FB_to_files", updated_users_from_FB_to_files);
    }

    // users_logs_counter

    if (preferences.isKey("users_logs_counter")) {
        users_logs_counter = preferences.getInt("users_logs_counter", 0);
        Serial.print("users_logs_counter exists. Value: ");
        Serial.println(users_logs_counter);
    }
    else {
        // The "id" key does not exist in preferences, so initialize it with a default value
        int defaultValue = 1;
        preferences.putInt("users_logs_counter", defaultValue);
        Serial.print("users_logs_counter did not exist. Initialized with default value: ");
        Serial.println(defaultValue);
    }
  
  if (preferences.isKey("users_logs_FB_counter")) {
        users_logs_FB_counter = preferences.getInt("users_logs_FB_counter", 0);
        Serial.print("users_logs_FB_counter exists. Value: ");
        Serial.println(users_logs_FB_counter);
    }
    else {
        // The "id" key does not exist in preferences, so initialize it with a default value
        int defaultValue = 1;
        preferences.putInt("users_logs_FB_counter", defaultValue);
        Serial.print("users_logs_FB_counter did not exist. Initialized with default value: ");
        Serial.println(defaultValue);
    }

    preferences.end();
}

void fingerPrintSensorClear() {
    for (int i = 0; i <= 162; ++i) {
        if (finger.deleteModel(i) == FINGERPRINT_OK) {
            Serial.println("Template deleted: " + String(i));
        }
        else {
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
    }
    else if (p == FINGERPRINT_PACKETRECIEVEERR) {
        //Serial.println("Communication error");
        return -2;
    }
    else if (p == FINGERPRINT_NOTFOUND) {
        //Serial.println("Did not find a match");
        return -1;
    }
    else {
        //Serial.println("Unknown error");
        return -2;
    }
    // found a match!
    //Serial.print("Found ID #"); Serial.print(finger.fingerID);
    //Serial.print(" with confidence of "); Serial.println(finger.confidence);
    return finger.fingerID;
}


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

    int ID_StartTime = millis();

    while (millis() - ID_StartTime < ID_Timeout || ID_StartTime == 0) {
        char customKey = customKeypad.getKey();
        if (customKey) {
            display.clearDisplay();
            display.setTextSize(1);
            display.setTextColor(WHITE);
            display.setCursor(0, 10);
            display.println("Please enter ID:");
            display.display();
            if (customKey == 'A' || customKey == 'B' || customKey == 'C' || customKey == 'D') {
                display.setCursor(0, 50);
                display.println("Illegal Value X");
                display.display();
                Serial.println("illegal value");
            }
            else if (customKey == '*') {
                if (ID_len == 0) {
                    display.setCursor(0, 50);
                    display.println("Nothing to undo");
                    display.display();
                    Serial.println("Nothing to undo");
                }
                else {
                    inputString = inputString.substring(0, inputString.length() - 1);
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
            else if (customKey == '#') {
                display.clearDisplay();
                display.setTextSize(1);
                display.setTextColor(WHITE);
                display.setCursor(0, 10);
                if (ID_len != 9) {
                    display.println("Invalid ID length");
                    display.setCursor(0, 25);
                    display.println("Try again!");
                    display.display();
                    Serial.println("invalid ID length");
                }
                else {
                    display.println("ID Fetched");
                    display.setCursor(0, 25);
                    display.println("Succesfully");
                    display.display();
                    Serial.println("ID sent to approval queue");
                    delay(2000);
                    return inputString;
                }
                inputString = "";
                ID_len = 0;
            }
            else {
                if (ID_len == 9) {
                    display.setCursor(0, 50);
                    display.println("Maximum length!");
                    display.display();
                    Serial.println("Maximum length excceded");
                }
                else {
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
    inputString = "";
    return inputString;
}

uint8_t getFingerprintEnroll() {
    int p = -1;
    display.clearDisplay();
    display.drawBitmap(34, 0, FinPr_scan_bits, FinPr_scan_width, FinPr_scan_height, WHITE);
    display.display();
    while (p != FINGERPRINT_OK) {
        p = finger.getImage();
        switch (p) {
        case FINGERPRINT_OK:
            //Serial.println("Image taken");
            display.clearDisplay();
            display.drawBitmap(34, 0, FinPr_valid_bits, FinPr_valid_width, FinPr_valid_height, WHITE);
            display.display();
            break;
        case FINGERPRINT_NOFINGER:
            //Serial.println(".");
            display.setTextSize(1); // Normal 2:2 pixel scale
            display.setTextColor(WHITE); // Draw white text
            display.setCursor(0, 0); // Start at top-left corner
            display.print(F("Registering"));
            display.display();
            break;
        case FINGERPRINT_PACKETRECIEVEERR:
            display.clearDisplay();
            display.drawBitmap(34, 0, FinPr_invalid_bits, FinPr_invalid_width, FinPr_invalid_height, WHITE);
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
        display.drawBitmap(34, 0, FinPr_valid_bits, FinPr_valid_width, FinPr_valid_height, WHITE);
        display.display();
        break;
    case FINGERPRINT_IMAGEMESS:
        display.clearDisplay();
        display.drawBitmap(34, 0, FinPr_invalid_bits, FinPr_invalid_width, FinPr_invalid_height, WHITE);
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
    display.setCursor(0, 0); // Start at top-left corner
    display.print(F("Remove"));
    display.setCursor(0, 20);
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
    display.drawBitmap(34, 0, FinPr_scan_bits, FinPr_scan_width, FinPr_scan_height, WHITE);
    display.display();
    while (p != FINGERPRINT_OK) {
        p = finger.getImage();
        switch (p) {
        case FINGERPRINT_OK:
            //Serial.println("Image taken");
            display.clearDisplay();
            display.drawBitmap(34, 0, FinPr_valid_bits, FinPr_valid_width, FinPr_valid_height, WHITE);
            display.display();
            break;
        case FINGERPRINT_NOFINGER:
            //Serial.println(".");
            display.setTextSize(1); // Normal 2:2 pixel scale
            display.setTextColor(WHITE); // Draw white text
            display.setCursor(0, 0); // Start at top-left corner
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
        display.drawBitmap(34, 0, FinPr_valid_bits, FinPr_valid_width, FinPr_valid_height, WHITE);
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
        display.drawBitmap(34, 0, FinPr_valid_bits, FinPr_valid_width, FinPr_valid_height, WHITE);
        display.display();
    }
    else if (p == FINGERPRINT_PACKETRECIEVEERR) {
        Serial.println("Communication error");
        return p;
    }
    else if (p == FINGERPRINT_ENROLLMISMATCH) {
        Serial.println("Fingerprints did not match");
        return p;
    }
    else {
        Serial.println("Unknown error");
        return p;
    }
    Serial.print("ID "); Serial.println(id);
    p = finger.storeModel(id);
    if (p == FINGERPRINT_OK) {
        //Serial.println("Stored!");
        display.clearDisplay();
        display.drawBitmap(34, 0, FinPr_valid_bits, FinPr_valid_width, FinPr_valid_height, WHITE);
        display.display();
        //confirmAdding(); to web
    }
    else if (p == FINGERPRINT_PACKETRECIEVEERR) {
        Serial.println("Communication error");
        return p;
    }
    else if (p == FINGERPRINT_BADLOCATION) {
        Serial.println("Could not store in that location");
        return p;
    }
    else if (p == FINGERPRINT_FLASHERR) {
        Serial.println("Error writing to flash");
        return p;
    }
    else {
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
void addUserToPendingUsersFile(int fingerprint_id, String id, char* currentTimeStr) {
    File file = SPIFFS.open(pending_users_path, "a");
    if (!file) {
        Serial.println("Failed to open file for writing");
        return;
    }

    Serial.println("addUserToPendingUsersFile---------- File opened successfully");

    DynamicJsonDocument doc(1024);
    JsonObject userObj = doc.createNestedObject();
    userObj["FingerPrintID"] = fingerprint_id;
    userObj["id"] = id;
    userObj["time"] = String(currentTimeStr);

    if (serializeJson(doc, file)) {
        Serial.println("addUserToPendingUsersFile----------- Data written to file successfully");
    }
    else {
        Serial.println("addUserToPendingUsersFile------------ Failed to write data to file");
    }

    file.close();

    file = SPIFFS.open(pending_users_path, "r");
    if (!file) {
        Serial.println("addUserToPendingUsersFile----------- Failed to open file for reading");
        return;
    }
    Serial.println("\n");
    Serial.println("addUserToPendingUsersFile------------- File Content:");
    while (file.available()) {
        Serial.write(file.read());
    }

    Serial.println("\n");

    file.close();
}

void addUserToFile(int fingerprint_id, String id, const char* currentTimeStr, const char* file_name) {
    DynamicJsonDocument doc(10000);
    File file;
    file = SPIFFS.open(file_name, "r");
    if (!file) {
        Serial.print("addUserToFile-------- Failed to open file for reading in ");
        Serial.print(file_name);
        Serial.println();
        return;
    }

    DeserializationError error = deserializeJson(doc, file);
    file.close();

    for (JsonObject userObj : doc.as<JsonArray>()) {
        Serial.print("addUserToFile-------");
        Serial.print(file_name);
        Serial.println();
        int fingerprint = userObj["FingerPrintID"].as<uint8_t>();
        if (fingerprint == fingerprint_id) {
            Serial.print("addUserToFile------- user found in file in ");
            Serial.print(file_name);
            Serial.println();
            return;
        }
    }

    file = SPIFFS.open(file_name, "a");
    if (!file) {
        Serial.print("addUserToFile-------Failed to open file for writing in ");
        Serial.println(file_name);
        Serial.println();
        return;
    }

    Serial.print("addUserToFile---------- File opened successfully in ");
    Serial.print(file_name);
    Serial.println();

    DynamicJsonDocument writedoc(10000);

    JsonObject userObj2 = writedoc.createNestedObject();
    userObj2["FingerPrintID"] = fingerprint_id;
    userObj2["id"] = id;
    userObj2["time"] = String(currentTimeStr);

    if (serializeJson(writedoc, file)) {
        Serial.print("addUserToFile----------- Data written to file successfully in ");
        Serial.print(file_name);
        Serial.println();
        updated_users_from_FB_to_files = updateUpdatingUserFromFBToArray(fingerprint_id);

        preferences.begin("myApp", false);
        preferences.putString("updated_users_from_FB_to_files", updated_users_from_FB_to_files);
        preferences.end();
    }
    else {
        Serial.print("addUserToFile------------ Failed to write data to file in ");
        Serial.print(file_name);
        Serial.println();
    }

    file.close();
}


// this tow function check if the user is in the rejected file or the approved file
bool isUserInFile(int targetFingerprint, const char* file_name) {
    DynamicJsonDocument doc(1024);
    File file = SPIFFS.open(file_name, "r");
    if (!file) {
        Serial.println("isUserInFile-------- Failed to open file for reading");
        return false;
    }

    DeserializationError error = deserializeJson(doc, file);
    file.close();

    if (error) {
        Serial.println("isUserInFile--------- Failed to read JSON");
        return false;
    }
  
    for (JsonObject userObj : doc.as<JsonArray>()) {
        int fingerprint = userObj["FingerPrintID"].as<uint8_t>();
        if (fingerprint == targetFingerprint) {
            Serial.print("isUserInFile----  user found with fingerprintid ");
      Serial.println(fingerprint);
            return true;
        }
    }
    return false; // User not found
}


void sendPendingUsersToFB() {
    File file = SPIFFS.open(pending_users_path, "r");
    if (!file) {
        Serial.println("sendPendingUsersToFB--------- Failed to read JSON from file");
        return;
    }

    Serial.println("\n");
    Serial.println("sendPendingUsersToFB------------- File Content:");
    while (file.available()) {
        Serial.write(file.read());
    }

    Serial.println("\n");

    file.close();
    DynamicJsonDocument doc(10000);
    file = SPIFFS.open(pending_users_path, "r");

    if (Firebase.ready()) {
        if (file.available()) {
            for (int i = 1; i <= current_sent_id_to_FB; i++) {
                while (file.read() != ']');
            }
        }
        while (file.available() && (current_sent_id_to_FB < id)) {
            DeserializationError error = deserializeJson(doc, file);
      Serial.println("serialzie output: ");
      serializeJsonPretty(doc, Serial);
            if (error) {
        Serial.println("here1");
                Serial.println("sendPendingUsersToFB-------- Failed to read JSON");
        file.close();
                return;
            }
      serializeJsonPretty(doc, Serial);
            String jsonString;
            serializeJson(doc, jsonString);
            if (jsonString.startsWith("[") && jsonString.endsWith("]")) {
                jsonString = jsonString.substring(1, jsonString.length() - 1);
            }
      Serial.println(jsonString);
            error = deserializeJson(doc, jsonString);
            if (error) {
        Serial.println("here1");
                Serial.println("sendPendingUsersToFB-------- Failed to read JSON");
        file.close();
                return;
            }
            JsonObject userObj = doc.as<JsonObject>();

            String documentPath = "PendingUsers/user" + userObj["FingerPrintID"].as<String>(); // id- fingerPrintID
      FirebaseJson content;

            content.set("fields/data/mapValue/fields/FingerPrintID/integerValue", userObj["FingerPrintID"].as<int>());
            content.set("fields/data/mapValue/fields/id/stringValue", userObj["id"].as<String>());
            content.set("fields/data/mapValue/fields/isPending/booleanValue", true);
            //content.set("fields/data/mapValue/fields/isApproved/booleanValue", false);
            content.set("fields/data/mapValue/fields/time/stringValue", userObj["time"].as<String>());
            //  content.set("fields/data/mapValue/fields/time/stringValue", currentTimeStr);
            // timestamp
            //content.set("fields/Time/timestampValue", "2014-10-02T15:01:23Z");

            Serial.println("adding_user");
            Serial.println("Create a document... ");

            if (Firebase.Firestore.createDocument(&fbdo, FIREBASE_PROJECT_ID, "", documentPath.c_str(), content.raw())) {
                Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());
                current_sent_id_to_FB++;
                preferences.begin("myApp", false);
                preferences.putInt("cuurent_pending_user_sent", current_sent_id_to_FB);
                preferences.end();
                Serial.println("current_sent_id_to_FB value:");
                Serial.println(current_sent_id_to_FB);
            }
            else {
                Serial.println(fbdo.errorReason());
            }
            file.read();
        }
    }
    file.close();
}

void UpdateUsersFileFromFB(bool isApprovedFile) {
    if (Firebase.ready()) {
        for (int i = 1; i <= current_sent_id_to_FB; i++) {
            if (!isUserUpdatingToFileFromFB(i)) {
                // we have to check if the user is in the approved collection in firebase
                String documentPath;
                String mask = "data";
                DynamicJsonDocument jsonArray(1024);
                JsonArray documents = jsonArray.to<JsonArray>();
                if (isApprovedFile) {
                    documentPath = "ApprovedUsers/user" + String(i);
                }
                else {
                    documentPath = "RejectedUsers/user" + String(i);
                }
                if (Firebase.Firestore.getDocument(&fbdo, FIREBASE_PROJECT_ID, "", documentPath.c_str(), mask.c_str())) {
                    Serial.println(fbdo.payload());

                    DynamicJsonDocument doc(1024);
                    deserializeJson(doc, fbdo.payload());
                    documents.add(doc);
                    JsonObject obj = doc.as<JsonObject>();

                    String id = obj["fields"]["data"]["mapValue"]["fields"]["id"].as<String>();
                    //int fingerprint_id = obj["fields"]["data"]["mapValue"]["fields"]["FingerPrintID"].as<int>();
                    String time_str = obj["fields"]["data"]["mapValue"]["fields"]["time"].as<String>();

                    Serial.print("ID: ");
                    Serial.println(id);
                    Serial.print("Fingerprint ID: ");
                    Serial.println(i);
                    Serial.print("Time: ");
                    Serial.println(time_str);
                    Serial.println();

                    if (isApprovedFile) {
                        addUserToFile(i, id, time_str.c_str(), approved_users_path);
                    }
                    else {
                        addUserToFile(i, id, time_str.c_str(), rejected_users_path);
                    }
                }
                else {
                    Serial.println(fbdo.errorReason());
                }
            }
        }
    }
}



void UpdateFireBase() {
    sendPendingUsersToFB();
    UpdateUsersFileFromFB(true);   // approved file
    UpdateUsersFileFromFB(false);  // rejected file
}

void printSPIFFSfiles() {
    File root = SPIFFS.open("/");
    File file = root.openNextFile();
    Serial.println("\n");
    while (file) {
        Serial.print("FILE: ");
        Serial.println(file.name());
        file = root.openNextFile();
    }
    Serial.println("\n");
}



void addPendingUserToFireBase(int fingerprint_id, String id, char* currentTimeStr) {
    if (Firebase.ready()) {

        String documentPath = "PendingUsers/user" + String(fingerprint_id); // id- fingerPrintID
    FirebaseJson content;

        content.set("fields/data/mapValue/fields/FingerPrintID/integerValue", fingerprint_id);
        content.set("fields/data/mapValue/fields/id/stringValue", id.c_str());
        content.set("fields/data/mapValue/fields/isPending/booleanValue", true);
        //content.set("fields/data/mapValue/fields/isApproved/booleanValue", is_approved);
        content.set("fields/data/mapValue/fields/time/stringValue", currentTimeStr);
        Serial.println("Create a document... ");

        if (Firebase.Firestore.createDocument(&fbdo, FIREBASE_PROJECT_ID, "", documentPath.c_str(), content.raw())) {
            Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());
      current_sent_id_to_FB++;
            preferences.begin("myApp", false);
            preferences.putInt("cuurent_pending_user_sent", current_sent_id_to_FB);
            preferences.end();
            Serial.println("current_sent_id_to_FB value:");
            Serial.println(current_sent_id_to_FB);
            return;
        }
        else {
            Serial.println(fbdo.errorReason());
        }
    }
}

String getValueFromDictionary(String key, String jsonString) {
    String extractedValue;

    int keyIndex = jsonString.indexOf(key);

    if (keyIndex != -1) {
        int startIndex = jsonString.indexOf(":", keyIndex) + 1;
        int endIndex = jsonString.indexOf("}", startIndex);

        if (startIndex != -1 && endIndex != -1) {
            extractedValue = jsonString.substring(startIndex, endIndex);
            extractedValue.trim(); // Remove leading/trailing whitespace if any
        }
    }
    Serial.println("Extracted value: " + extractedValue);
    return extractedValue;
}

bool isUserExistInFBList(int id, char* currentTimeStr, bool list) {
    String documentPath;
    if (list) {
        documentPath = "ApprovedUsers/user" + String(id);
    }
    else {
        documentPath = "RejectedUsers/user" + String(id);
    }
    if (Firebase.ready()) {
        String mask = "data";
        Serial.println("Get a document... ");

        if (Firebase.Firestore.getDocument(&fbdo, FIREBASE_PROJECT_ID, "", documentPath.c_str(), mask.c_str())) {
            Serial.println(fbdo.payload());
            DynamicJsonDocument doc(10000);
            // You can use a String as your JSON input.
            // WARNING: the string in the input  will be duplicated in the JsonDocument.
            deserializeJson(doc, fbdo.payload());
            JsonObject obj = doc.as<JsonObject>();
            Serial.println(obj["fields"]["data"]["mapValue"]["fields"]["id"].as<String>());
            Serial.println(obj["fields"]["data"]["mapValue"]["fields"]["FingerPrintID"].as<String>());


            String userID = getValueFromDictionary(String("stringValue"), obj["fields"]["data"]["mapValue"]["fields"]["id"].as<String>());// digits

            if (list) {
                addUserToFile(id, userID, currentTimeStr, approved_users_path); // write to approved file
            }
            else {
                addUserToFile(id, userID, currentTimeStr, rejected_users_path); // write to approved file
            }
            return true;
        }
        else {
            Serial.println(fbdo.errorReason());
            return false;
        }
    }
    return false;
}

void addLogToFile(int fingerprint_id, bool is_approved, bool is_pending, char* time_str) {
    File file = SPIFFS.open(users_logs_path, "a");
    if (!file) {
        Serial.println("addLogToFile---------- Failed to open file for writing");
        return;
    }

    Serial.println("addLogToFile---------- File opened successfully");

    DynamicJsonDocument doc(1024);
    JsonObject userObj = doc.createNestedObject();

    userObj["FingerPrintID"] = fingerprint_id;
    if (is_approved) {
        userObj["status"] = "approved";
    }
    else if (!is_approved && !is_pending) {
        userObj["status"] = "rejected";
    }
    else {
        userObj["status"] = "pending";
    }
    userObj["time"] = String(time_str);

    if (serializeJson(doc, file)) {
        Serial.println("addLogToFile----------- Data written to file successfully");
    users_logs_counter++;
        preferences.begin("myApp", false);
        preferences.putInt("users_logs_counter", users_logs_counter);
        preferences.end();
        Serial.println("users_logs_counter value:");
        Serial.println(users_logs_counter);
    }
    else {
        Serial.println("addLogToFile------------ Failed to write data to file");
    }

    file.close();
}

void sendUsersLogsToFB() {
    DynamicJsonDocument doc(10000);
    File file = SPIFFS.open(users_logs_path, "r");

    if (Firebase.ready()) {
        if (file.available()) {
            for (int i = 1; i <= users_logs_FB_counter; i++) {
                while (file.read() != ']');
            }
        }
        while (file.available()) {
            DeserializationError error = deserializeJson(doc, file);
            if (error) {
                Serial.println("sendUsersLogsToFB-------- Failed to read JSON");
                return;
            }
            String jsonString;
            serializeJson(doc, jsonString);
            if (jsonString.startsWith("[") && jsonString.endsWith("]")) {
                jsonString = jsonString.substring(1, jsonString.length() - 1);
            }
            error = deserializeJson(doc, jsonString);
            if (error) {
                Serial.println("sendUsersLogsToFB-------- Failed to read JSON");
                return;
            }
            JsonObject userObj = doc.as<JsonObject>();

            String documentPath = "UsersLogs/log" + String(users_logs_FB_counter);
      FirebaseJson content;

            content.set("fields/data/mapValue/fields/FingerPrintID/integerValue", userObj["FingerPrintID"].as<int>());
            content.set("fields/data/mapValue/fields/status/stringValue", userObj["status"].as<String>());
            content.set("fields/data/mapValue/fields/time/stringValue", userObj["time"].as<String>());

            Serial.println("uploading logs to FB");
            Serial.println("Create a document... ");

            if (Firebase.Firestore.createDocument(&fbdo, FIREBASE_PROJECT_ID, "", documentPath.c_str(), content.raw())) {
                Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());
                users_logs_FB_counter++;
                preferences.begin("myApp", false);
                preferences.putInt("users_logs_FB_counter", users_logs_FB_counter);
                preferences.end();
                Serial.println("users_logs_FB_counter value:");
                Serial.println(users_logs_FB_counter);
            }
            else {
                Serial.println(fbdo.errorReason());
            }
            file.read();
        }
    }
    file.close();
}

void loop() {
    Serial.println("Waiting for valid finger...");
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
    struct tm timeinfo;
    char currentTimeStr[20] = "";
    
  if (hasConnected) {
    if (!getLocalTime(&timeinfo)) {
      Serial.println("Failed to obtain time");
    }
    else {
      strftime(currentTimeStr, sizeof(currentTimeStr), "%Y-%m-%d %H:%M:%S", &timeinfo);
    }
  }
  
    bool is_pending = false;
    bool is_approved = false;

    //Didn't find a match
    if (FingerID == -1) {

        setColor(255, 255, 0);
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(0, 10);
        display.println("Access Denied! You");
        display.setCursor(0, 30);
        display.println("are not registered.");
        display.display();

        delay(4000);

        Serial.println("Enter your ID in keypad");
        id_str = GetIdFromKeypad();

        if (id_str == "") {
            setColor(255, 50, 0);
            display.clearDisplay();
            display.setTextSize(1);
            display.setTextColor(WHITE);
            display.setCursor(0, 10);
            // Display static text
            display.println("ID Timeout");
            display.setCursor(0, 30);
            display.println("Please try again");
            display.display();
            delay(2000);
        }
        else {


            fingerPrintID = getFingerprintEnroll();

            if (fingerPrintID == fingerprint_timeout) {
                setColor(255, 50, 0);
                display.clearDisplay();
                display.setTextSize(1);
                display.setTextColor(WHITE);
                display.setCursor(0, 10);
                // Display static text
                display.println("FP Timeout");
                display.setCursor(0, 30);
                display.println("Please try again");
                display.display();
                delay(2000);
            }

            if (fingerPrintID != FINGERPRINT_OK) {
                setColor(255, 50, 0);
                display.clearDisplay();
                display.setTextSize(1);
                display.setTextColor(WHITE);
                display.setCursor(0, 10);
                // Display static text
                display.println("Registration Error");
                display.setCursor(0, 30);
                display.println("Please try again");
                display.display();
                delay(1000);
            }

            else {
                printSPIFFSfiles();
                if (id > 0 && id <= 162 && fingerPrintID == FINGERPRINT_OK) {
                    /*if (!getLocalTime(&timeinfo)) {
                        Serial.println("Failed to obtain time");
                        //return;
                    }
                    else {
                        strftime(currentTimeStr, sizeof(currentTimeStr), "%Y-%m-%d %H:%M:%S", &timeinfo);
                    }*/
                    addUserToPendingUsersFile(id, id_str, currentTimeStr);
                    Serial.println("Files IN SPIFFS");
                    printSPIFFSfiles();
                    display.clearDisplay();
                    display.setTextSize(1);
                    display.setTextColor(WHITE);
                    display.setCursor(0, 10);
                    // Display static text
                    display.println("User Registered");
                    display.setCursor(0, 30);
                    display.println("Succesfully");
                    display.display();
                    delay(3000);
                    if (Firebase.ready() && (WiFi.status() == WL_CONNECTED)) { // upload the data to the firebase to the pinding users
                        if ((current_sent_id_to_FB + 1) == id) {
                            addPendingUserToFireBase(id, id_str, currentTimeStr);
                        }
                        else {
                            sendPendingUsersToFB();
                        }
                    }
                    else {
                        if (millis() - reconnectTime > reconnectionTimeout) {
                            reconnectTime = millis();
                            connectToWifi();
                        }
                    }
                    id++;
                    preferences.begin("myApp", false);
                    preferences.putInt("id", id);
                    preferences.end();
                    //addLogToFile(FingerID, false, true, currentTimeStr);
                }
                else {
                    Serial.println("there is no avalibe place in the system\n");
                }
            }
        }
    } // end if fingerif not found

    // found image
    else if (FingerID > 0) {
        if (isUserInFile(FingerID, approved_users_path)) { // check if is in the approved file
            setColor(0, 255, 0);
            display.clearDisplay();
            display.println("Welcome!\n");
            display.display();
            delay(1000);
            addLogToFile(FingerID, true, false, currentTimeStr);

        }
        else if (isUserInFile(FingerID, rejected_users_path)) { // check if is  in the rejected file
            setColor(255, 0, 0);
            display.clearDisplay();
            display.println("Rejected!\n");
            display.display();
            delay(1000);
            addLogToFile(FingerID, false, false, currentTimeStr);
        }
        // check if user have been approved or rejected
        else if (Firebase.ready() && (WiFi.status() == WL_CONNECTED)) { // upload the data to the firebase to the pinding users
            if (isUserExistInFBList(FingerID, currentTimeStr, true)) { // this function insert the user to the approved users file
                setColor(0, 255, 0);
                display.clearDisplay();
                display.println("Welcome!\n");
                display.display();
                delay(1000);
                addLogToFile(FingerID, true, false, currentTimeStr);
            }
            // isUserExistInFBList cehck in firebase and updates the files
            else if (isUserExistInFBList(FingerID, currentTimeStr, false)) { // false for rejected
                setColor(255, 0, 0);
                display.clearDisplay();
                display.println("Rejected!\n");
                display.display();
                delay(1000);
                addLogToFile(FingerID, false, false, currentTimeStr);
            }
        }
        else { // user is waiting in the pendig list
            setColor(255, 0, 255);
            display.clearDisplay();
            display.setTextSize(1);
            display.setTextColor(WHITE);
            display.setCursor(0, 10);
            // Display static text
            display.println("Access Denied, you");
            display.setCursor(0, 30);
            display.println("are in the waiting list");
            display.display();
            delay(1000);
            Serial.println("\nYou are in the pending list mannn!, wait until the ceo accept your request\n");
            addLogToFile(FingerID, false, true, currentTimeStr);
        }
    }

    //addLogToFile(id, is_approved, is_pending, currentTimeStr);

    if ((millis() - printFileContentTime) > printFileContentTimeOut) {
        printFileContentTime = millis();
        printFileContent(approved_users_path);
        printFileContent(rejected_users_path);
        printFileContent(pending_users_path);
        printFileContent(users_logs_path);

        if (WiFi.status() == WL_CONNECTED) {
            sendUsersLogsToFB();
        }
    }

    if (!(WiFi.status() == WL_CONNECTED) && ((millis() - reconnectTime) > reconnectionTimeout)) {
        reconnectTime = millis();
        connectToWifi();
    }
    if ((WiFi.status() == WL_CONNECTED) && ((millis() - UpdateTime) > UpdateTimeout)) {
        UpdateTime = millis();
        UpdateFireBase();
    }
    delay(1000); // Delay to prevent rapid fingerprint reading
}
