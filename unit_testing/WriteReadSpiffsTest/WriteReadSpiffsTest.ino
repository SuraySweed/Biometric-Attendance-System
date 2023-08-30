#include "SPIFFS.h"

void setup() {
 
  Serial.begin(115200);
  Serial.println();  
 
  bool success = SPIFFS.begin();
  SPIFFS.format();
  
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
 
}
 
void loop() {}
