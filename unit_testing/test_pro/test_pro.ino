/*
Developed by Electronics Innovation
Project: Portable IoT based Fingerprint Biometric Attendance System with NodeMCU.
Electronics Innovation - www.electronicsinnovation.com
GitHub - https://github.com/VeeruSubbuAmi
YouTube - http://bit.ly/Electronics_Innovation
Upload date: 18 September 2019
Libraries:
Adafruit GFX:- https://github.com/adafruit/Adafruit-GFX-Library
Adafruit SSD1306:- https://github.com/adafruit/Adafruit_SSD1306
Adafruit-Fingerprint-Sensor-Library:- https://github.com/adafruit/Adafruit-Fingerprint-Sensor-Library
This sketch is developed to capture fingerprint data to compare the existing data and mark attendance on the server over the Internet.
Note: 1)Connect the circuit as shown in the circuit diagram.
2) Download and install all the required libraries.
All the Links are given above.
*/
//*******************************libraries********************************
#include <SPI.h>
#include <Wire.h>
#include <WiFi.h>
//#include &lt;SoftwareSerial.h&gt; // https://youtu.be/oq3_xy2rCyk
#include <WebServer.h>
#include <HTTPClient.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Fingerprint.h>
#include "helper.h"
#include "credentialsData.h"

// Pins connected to the fingerprint sensor
#define FINGERPRINT_RX_PIN 16
#define FINGERPRINT_TX_PIN 17

HardwareSerial fingerSerial(1);  // Use hardware serial port 1
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&fingerSerial);
//************************************************************************
// Declaration for SSD1306 display connected using software I2C
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET 0 // Reset pin # (or -1 if sharing Arduino reset pin)

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

String postData ; // post array that will be send to the website
//String link_s = "http://192.168.1.14/biometricattendance/getdata.php"; //computer IP or the server domain
int FingerID = 0; // The Fingerprint ID from the scanner
uint8_t id;


void connectToWiFi();
void DisplayFingerprintID();
void SendFingerprintID(int finger);
int getFingerprintID();
void ChecktoDeleteID();
void connectToWiFi();
void DisplayFingerprintID();
void SendFingerprintID(int finger);
int getFingerprintID();
void ChecktoDeleteID();
uint8_t deleteFingerprint(int id);
void ChecktoAddID();
uint8_t getFingerprintEnroll();
void confirmAdding();

//************************************************************************
void setup() {
  Serial.begin(115200);
  //-----------initiate OLED display-------------
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  // you can delet these three lines if you don't want to get the Adfruit logo appear
  display.display();
  delay(2000); // Pause for 2 seconds
  display.clearDisplay();
  
  
  //connectToWiFi();

  
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
  //------------*test the connection*------------
  //SendFingerprintID( FingerID );
}
//***********************************************************************
void loop() {
  //check if there's a connection to WiFi or not
  if(WiFi.status() != WL_CONNECTED){
    connectToWiFi();
  }
  //If there no fingerprint has been scanned return -1 or -2 if there an error or 0 if there nothing, The ID start form 1 to 127
  FingerID = getFingerprintID(); // Get the Fingerprint ID from the Scanner
  delay(50); //don't need to run this at full speed.
  
  DisplayFingerprintID();
  ChecktoAddID();
  ChecktoDeleteID();
}

//********************connect to the WiFi******************
void connectToWiFi(){
  WiFi.mode(WIFI_OFF); //Prevents reconnection issue (taking too long to connect)
  delay(1000);
  WiFi.mode(WIFI_STA);
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  display.clearDisplay();
  display.setTextSize(1); // Normal 1:1 pixel scale
  display.setTextColor(WHITE); // Draw white text
  display.setCursor(0, 0); // Start at top-left corner
  display.print(F("Connecting to \n"));
  display.setCursor(0, 50);
  display.setTextSize(2);
  display.print(ssid);
  display.drawBitmap( 73, 10, Wifi_start_bits, Wifi_start_width, Wifi_start_height, WHITE);
  display.display();
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("");
  Serial.println("\nConnected");
  display.clearDisplay();
  display.setTextSize(2); // Normal 1:1 pixel scale
  display.setTextColor(WHITE); // Draw white text
  display.setCursor(8, 0); // Start at top-left corner
  display.print(F("Connected \n"));
  display.drawBitmap( 33, 15, Wifi_connected_bits, Wifi_connected_width, Wifi_connected_height, WHITE);
  display.display();
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP()); //IP address assigned to your ESP
}


  //************Display the fingerprint ID state on the OLED*************
void DisplayFingerprintID(){
  //Fingerprint has been detected
  if (FingerID > 0) {
    display.clearDisplay();
    display.drawBitmap( 34, 0, FinPr_valid_bits, FinPr_valid_width, FinPr_valid_height, WHITE);
    display.display();
    SendFingerprintID( FingerID ); // Send the Fingerprint ID to the website.
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
//************send the fingerprint ID to the website*************
void SendFingerprintID( int finger ){
  HTTPClient http; //Declare object of class HTTPClient
  //Post Data
  postData = "FingerID=" + String(finger); // Add the Fingerprint ID to the Post array in order to send it
  // Post methode
  http.begin(link_s); //initiate HTTP request, put your Website URL or Your Computer IP
  http.addHeader("Content-Type", "application/x-www-form-urlencoded"); //Specify content-type header
  int httpCode = http.POST(postData); //Send the request
  String payload = http.getString(); //Get the response payload
  Serial.println(httpCode); //Print HTTP return code
  Serial.println(payload); //Print request response payload
  Serial.println(postData); //Post Data
  Serial.println(finger); //Print fingerprint ID
  if (payload.substring(0, 5) == "login") {
    String user_name = payload.substring(5);
    // Serial.println(user_name);
    display.clearDisplay();
    display.setTextSize(2); // Normal 2:2 pixel scale
    display.setTextColor(WHITE); // Draw white text
    display.setCursor(15,0); // Start at top-left corner
    display.print(F("Welcome"));
    display.setCursor(0,20);
    display.print(user_name);
    display.display();
  }
  else if (payload.substring(0, 6) == "logout") {
    String user_name = payload.substring(6);
    // Serial.println(user_name);
    display.clearDisplay();
    display.setTextSize(2); // Normal 2:2 pixel scale
    display.setTextColor(WHITE); // Draw white text
    display.setCursor(10,0); // Start at top-left corner
    display.print(F("Good Bye"));
    display.setCursor(0,20);
    display.print(user_name);
    display.display();
  }
  delay(1000);
  postData = "";
  http.end(); //Close connection
}
//********************Get the Fingerprint ID******************
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
//******************Check if there a Fingerprint ID to delete******************
void ChecktoDeleteID(){
  HTTPClient http; //Declare object of class HTTPClient
  //Post Data
  postData = "DeleteID=check"; // Add the Fingerprint ID to the Post array in order to send it
  // Post methode
  http.begin(link_s); //initiate HTTP request, put your Website URL or Your Computer IP
  http.addHeader("Content-Type", "application/x-www-form-urlencoded"); //Specify content-type header
  int httpCode = http.POST(postData); //Send the request
  String payload = http.getString(); //Get the response payload
  if (payload.substring(0, 6) == "del-id") {
    String del_id = payload.substring(6);
    Serial.println(del_id);
    deleteFingerprint( del_id.toInt() );
  }
  http.end(); //Close connection
}
  //******************Delete Finpgerprint ID*****************
uint8_t deleteFingerprint( int id) {
  uint8_t p = -1;
  p = finger.deleteModel(id);
  if (p == FINGERPRINT_OK) {
    //Serial.println("Deleted!");
    display.clearDisplay();
    display.setTextSize(2); // Normal 2:2 pixel scale
    display.setTextColor(WHITE); // Draw white text
    display.setCursor(0,0); // Start at top-left corner
    display.print(F("Deleted!\n"));
    display.display();
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    //Serial.println("Communication error");
    display.clearDisplay();
    display.setTextSize(1); // Normal 1:1 pixel scale
    display.setTextColor(WHITE); // Draw white text
    display.setCursor(0,0); // Start at top-left corner
    display.print(F("Communication error!\n"));
    display.display();
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    //Serial.println("Could not delete in that location");
    display.clearDisplay();
    display.setTextSize(1); // Normal 1:1 pixel scale
    display.setTextColor(WHITE); // Draw white text
    display.setCursor(0,0); // Start at top-left corner
    display.print(F("Could not delete in that location!\n"));
    display.display();
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    //Serial.println("Error writing to flash");
    display.clearDisplay();
    display.setTextSize(1); // Normal 1:1 pixel scale
    display.setTextColor(WHITE); // Draw white text
    display.setCursor(0,0); // Start at top-left corner
    display.print(F("Error writing to flash!\n"));
    display.display();
    return p;
  } else {
    //Serial.print("Unknown error: 0x"); Serial.println(p, HEX);
    display.clearDisplay();
    display.setTextSize(2); // Normal 2:2 pixel scale
    display.setTextColor(WHITE); // Draw white text
    display.setCursor(0,0); // Start at top-left corner
    display.print(F("Unknown error:\n"));
    display.display();
    return p;
  }
}
//******************Check if there a Fingerprint ID to add******************
void ChecktoAddID(){
  HTTPClient http; //Declare object of class HTTPClient
  //Post Data
  postData = "Get_Fingerid=get_id"; // Add the Fingerprint ID to the Post array in order to send it
  // Post methode
  http.begin(link_s); //initiate HTTP request, put your Website URL or Your Computer IP
  http.addHeader("Content-Type", "application/x-www-form-urlencoded"); //Specify content-type header
  int httpCode = http.POST(postData); //Send the request
  String payload = http.getString(); //Get the response payload
  if (payload.substring(0, 6) == "add-id") {
    String add_id = payload.substring(6);
    Serial.println(add_id);
    id = add_id.toInt();
    getFingerprintEnroll();
  }
  http.end(); //Close connection
}
//******************Enroll a Finpgerprint ID*****************
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
    confirmAdding();
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
//******************Check if there a Fingerprint ID to add******************
void confirmAdding(){
  HTTPClient http; //Declare object of class HTTPClient
  //Post Data
  postData = "confirm_id=" + String(id); // Add the Fingerprint ID to the Post array in order to send it
  // Post methode
  http.begin(link_s); //initiate HTTP request, put your Website URL or Your Computer IP
  http.addHeader("Content-Type", "application/x-www-form-urlencoded"); //Specify content-type header
  int httpCode = http.POST(postData); //Send the request
  String payload = http.getString(); //Get the response payload
  display.clearDisplay();
  display.setTextSize(1.5); // Normal 1:1 pixel scale
  display.setTextColor(WHITE); // Draw white text
  display.setCursor(0,0); // Start at top-left corner
  display.print(payload);
  display.display();
  delay(1000);
  Serial.println(payload);
  http.end(); //Close connection
}

