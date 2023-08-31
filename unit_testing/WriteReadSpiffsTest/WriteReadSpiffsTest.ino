#include "SPIFFS.h"
#include <ArduinoJson.h>
#include <TimeLib.h> 

int x = 1;

void setup() {
 
  Serial.begin(115200);
  Serial.println();  

  bool success = SPIFFS.begin();
  SPIFFS.format();

  /*
  if (success) {
    Serial.println("File system mounted with success");
  } else {
    Serial.println("Error mounting the file system");
    return;
  }
 
  File file = SPIFFS.open("/file.txt", "w");
 
  if (!file) {
    Serial.println("Error opening file for writing");
    return;
  }
 
  int bytesWritten = file.print("TEST SPIFFS");
 
  if (bytesWritten > 0) {
    Serial.println("File was written");
    Serial.println(bytesWritten);
  } else {
    Serial.println("File write failed");
  }

  
  
  file.close();

  file = SPIFFS.open("/file.txt", "r");
  if(!file){
    Serial.println("Failed to open file for reading");
    return;
  }
  
  Serial.println("File Content:");
  while(file.available()){
    Serial.write(file.read());
  }
  file.close();
  */
}

 void saveUserDataToFile(uint8_t fingerprint_id, String id, bool is_appending, bool is_approved, time_t curr_time) {
    File file = SPIFFS.open("/users_data.json", "w");
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


    file = SPIFFS.open("/users_data.json", "r");
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
void loop() {
  x++;

  if (x == 3) {
    saveUserDataToFile(3, "322827239", true, false, now());
  }
}
