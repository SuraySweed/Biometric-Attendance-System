#include <ArduinoJson.h>
#include "SPIFFS.h"

const char* file_name = "/data.json";

void setup() {
    Serial.begin(115200);
     bool success = SPIFFS.begin();  
    DynamicJsonDocument doc(1024);
    File file;
    file = SPIFFS.open(file_name, "r");
    
    String jsonString;
    
    
     while (file.available()) {
      DeserializationError error = deserializeJson(doc, file);
      serializeJson(doc, jsonString);
      if (jsonString.startsWith("[") && jsonString.endsWith("]")) {
        jsonString = jsonString.substring(1, jsonString.length() - 1);
      }
      Serial.println(jsonString);

      error = deserializeJson(doc, jsonString);
      if (error) {
          Serial.println("sendWaitingUsersToFirestore-------- Failed to read JSON");
          return;
       }
       JsonObject userObj = doc.as<JsonObject>();
        Serial.println("userobj id");
        Serial.println(userObj["FingerPrintID"].as<String>());
      
      file.read();
      jsonString ="";
    }
    file.close();
}

void loop() {
  

}
