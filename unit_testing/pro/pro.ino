#include <Adafruit_Fingerprint.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "helper.h"

// Pins connected to the fingerprint sensor
#define FINGERPRINT_RX_PIN 16
#define FINGERPRINT_TX_PIN 17

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

HardwareSerial fingerSerial(1);  // Use hardware serial port 1
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&fingerSerial);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

uint8_t id;


void setup() {
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

  delay(1000);
  display.clearDisplay();

  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  // Display static text
  display.println("Hey There");
  display.display(); 

  Serial.begin(115200);
  fingerSerial.begin(57600, SERIAL_8N1, FINGERPRINT_RX_PIN, FINGERPRINT_TX_PIN);

  finger.begin(57600); // Initialize the fingerprint sensor

  delay(5);
  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1) { delay(1); }
  }
  
  Serial.println("Fingerprint sensor initialized");

  finger.getTemplateCount(); // Get the number of stored templates
  Serial.print("Sensor contains "); Serial.print(finger.templateCount); Serial.println(" templates");
}


void loop() {
  if (Serial.available() > 0) {
    int userInput = Serial.read() - '0'; // Convert ASCII to integer
    
    if (userInput == 1) {
      Serial.println("Enter fingerprint ID number:");
      while (!Serial.available());
      
      int fingerprintID = Serial.parseInt();
      enrollFingerprint(fingerprintID);
    } else if (userInput == 2) {
      checkFingerprint();
    }
  }
}

void enrollFingerprint(int id) {
  Serial.println("Place your finger on the sensor for enrollment...");
  
  while (!finger.getImage());
  
  int result = finger.image2Tz(1);
  if (result != FINGERPRINT_OK) {
    Serial.println("Error converting image to template");
    return;
  }
  
  Serial.println("Remove your finger");
  delay(2000);
  
  Serial.println("Place your finger again");
  while (!finger.getImage());
  
  result = finger.image2Tz(2);
  if (result != FINGERPRINT_OK) {
    Serial.println("Error converting image to template");
    return;
  }
  
  result = finger.createModel();
  if (result != FINGERPRINT_OK) {
    Serial.println("Error creating fingerprint model");
    return;
  }
  
  result = finger.storeModel(id);
  if (result == FINGERPRINT_OK) {
    Serial.println("Fingerprint enrolled successfully!");
  } else {
    Serial.println("Error storing fingerprint");
  }
}

void checkFingerprint() {
  Serial.println("Place your finger on the sensor for verification...");
  
  while (!finger.getImage());
  
  int result = finger.image2Tz();
  if (result != FINGERPRINT_OK) {
    Serial.println("No fingerprint found");
    return;
  }
  
  result = finger.fingerFastSearch();
  if (result == FINGERPRINT_OK) {
    Serial.println("Fingerprint found!");
    Serial.print("ID: "); Serial.println(finger.fingerID);
    Serial.print("Confidence: "); Serial.println(finger.confidence);
  } else {
    Serial.println("Fingerprint not found");
  }
}