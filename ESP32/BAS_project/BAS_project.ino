#include <WiFi.h>
#include <FirebaseESP32.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include "SECRETS.h"

#define API_KEY "AIzaSyCQOPaLIyCOJI71ASLu-DvJTlsbGsqRupA"

const char* storageBucket = "gs://biometric-attendance-sys-1deca.appspot.com";
const char* filePath = "/example.txt";

void setup() {
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

void loop() {
  
}

void uploadFile() {
  Serial.println("Uploading file...");

  HTTPClient http;
  String url = "https://firebasestorage.googleapis.com/v0/b/" + String(storageBucket) + "/o" + filePath +
               "?key=" + API_KEY;
  
  // Set the file content type
  http.addHeader("Content-Type", "text/plain");

  // Replace this with your file content or logic to read file content
  String fileContent = "Hello, Firebase!";

  int httpResponseCode = http.POST(url, fileContent);

  if (httpResponseCode > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    } else {
    Serial.println("Error in uploading file");
  }
  http.end();
}

void downloadFile() {
  Serial.println("Downloading file...");

  HTTPClient http;

  String url = "https://firebasestorage.googleapis.com/v0/b/" + String(storageBucket) + "/o" + filePath +
               "?alt=media&key=" + API_KEY;

  http.begin(url);

  int httpResponseCode = http.GET();
  if (httpResponseCode == HTTP_CODE_OK) {
    String responseBody = http.getString();
    Serial.println("File Content:");
    Serial.println(responseBody);
  } else {
    Serial.print("HTTP Error: ");
    Serial.println(httpResponseCode);
    }

  http.end();
}

void updateFile() {
  Serial.println("Updating file...");

  HTTPClient http;
  String url = "https://firebasestorage.googleapis.com/v0/b/" + String(storageBucket) + "/o" + filePath +
               "?key=" + API_KEY;

  // Set the file content type
  http.addHeader("Content-Type", "text/plain");

  // Replace this with your updated file content
  String updatedContent = "Updated content!";

  int httpResponseCode = http.PUT(url, updatedContent);

  if (httpResponseCode > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
  } else {
    Serial.println("Error in updating file");
  }

  http.end();
}


