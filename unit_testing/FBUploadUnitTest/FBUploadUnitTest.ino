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
      Serial.println(jsonString);
          file.read();
      jsonString ="";
    }
    file.close();
}

void loop() {
  

}
