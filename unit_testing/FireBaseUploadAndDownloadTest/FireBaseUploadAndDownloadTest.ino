#include "WiFi.h"
#include "ArduinoJson.h"
#include "SPIFFS.h"
#include <Firebase_ESP_Client.h>
#include "SECRETS.h"
#include <addons/TokenHelper.h>

#define API_KEY "AIzaSyCQOPaLIyCOJI71ASLu-DvJTlsbGsqRupA"

const char* storageBucket = "gs://biometric-attendance-sys-1deca.appspot.com";
//const char* filePath = "/data_users.json";

FirebaseData firebaseData;

void setup() {
    // Initialize Wi-Fi
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    
    // Initialize Firebase
    Firebase.begin(API_KEY, storageBucket);
}

void loop() {
  // Upload a file
  uploadFile("example.txt", "Hello, Firebase Cloud Storage!");

  // Download and print the uploaded file
  String downloadedContent = downloadFile("example.txt");
  Serial.println("Downloaded Content:");
  Serial.println(downloadedContent);

  delay(5000); // Wait for 5 seconds before next loop iteration
}

void uploadFile(const String& fileName, const String& fileContent) {
  String storagePath = "/" + fileName;

  if (Firebase.putString(firebaseData, storagePath, fileContent)) {
    Serial.println("File uploaded successfully!");
  } else {
    Serial.println("File upload failed.");
  }
}

String downloadFile(const String& fileName) {
  String storagePath = "/" + fileName;

  if (Firebase.getString(firebaseData, storagePath)) {
    return firebaseData.stringData();
  } else {
    Serial.println("File download failed.");
    return "";
  }
}



/*
//Define Firebase Data objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig configF;

bool taskCompleted = false;
bool signupOK = false;

void initWiFi(){
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
}

void initSPIFFS(){
  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    ESP.restart();
  }
  else {
    delay(500);
    Serial.println("SPIFFS mounted successfully");
  }
}

void writeJsonFile() {
  DynamicJsonDocument jsonDoc(1024);
  jsonDoc["key1"] = "value1";
  jsonDoc["key2"] = "value2";
  jsonDoc["key3"] = 42;

  // Open the file in write mode
  File file = SPIFFS.open(filePath, "w");
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }

  // Serialize the JSON document to the file
  serializeJson(jsonDoc, file);

  // Close the file
  file.close();

  Serial.println("JSON file written to SPIFFS");
}

void setup() {
  Serial.begin(115200);
  
  initWiFi();
  initSPIFFS();

  writeJsonFile();
  
  configF.api_key = API_KEY;
  //Assign the user sign in credentials
  if (Firebase.signUp(&configF, &auth, "", "")){
    Serial.println("ok");
    signupOK = true;
  }
  else {
   Serial.printf("%s\n", configF.signer.signupError.message.c_str());
  }
  //Assign the callback function for the long running token generation task
  configF.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

  Firebase.begin(&configF, &auth);
  Firebase.reconnectWiFi(true);
}

void loop() {
  if (Firebase.ready() && !taskCompleted && signupOK){
    taskCompleted = true;
    Serial.print("Uploading picture... ");

    //MIME type should be valid to avoid the download problem.
    //The file systems for flash and SD/SDMMC can be changed in FirebaseFS.h.
    if (Firebase.Storage.upload(&fbdo, STORAGE_BUCKET_ID, filePath, mem_storage_type_flash, filePath, "application/json")){
      Serial.printf("\nDownload URL: %s\n", fbdo.downloadURL().c_str());
  }
  else{
    Serial.println(fbdo.errorReason());
  }
}

}*/
