
/**
 * Created by K. Suwatchai (Mobizt)
 *
 * Email: k_suwatchai@hotmail.com
 *
 * Github: https://github.com/mobizt/Firebase-ESP-Client
 *
 * Copyright (c) 2023 mobizt
 *
 */

// This example shows how to get a document from a document collection. This operation required Email/password, custom or OAUth2.0 authentication.

#include <Arduino.h>
#if defined(ESP32) || defined(ARDUINO_RASPBERRY_PI_PICO_W)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif

#include <Firebase_ESP_Client.h>
#include <ArduinoJson.h>

// Provide the token generation process info.
#include <addons/TokenHelper.h>

/* 1. Define the WiFi credentials */
#define WIFI_SSID "Boran"
#define WIFI_PASSWORD "zeuv8379"

/* 2. Define the API Key */
#define API_KEY "AIzaSyCQOPaLIyCOJI71ASLu-DvJTlsbGsqRupA"

/* 3. Define the project ID */
#define FIREBASE_PROJECT_ID "biometric-attendance-sys-1deca"

/* 4. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "biometproj@gmail.com"
#define USER_PASSWORD "12345678project"

// Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

bool taskCompleted = false;

unsigned long dataMillis = 0;

#if defined(ARDUINO_RASPBERRY_PI_PICO_W)
WiFiMulti multi;
#endif

void setup()
{

    Serial.begin(115200);

#if defined(ARDUINO_RASPBERRY_PI_PICO_W)
    multi.addAP(WIFI_SSID, WIFI_PASSWORD);
    multi.run();
#else
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
#endif

    Serial.print("Connecting to Wi-Fi");
    unsigned long ms = millis();
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(300);
#if defined(ARDUINO_RASPBERRY_PI_PICO_W)
        if (millis() - ms > 10000)
            break;
#endif
    }
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();


    /* Assign the api key (required) */
    config.api_key = API_KEY;

    /* Assign the user sign in credentials */
    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;

    // The WiFi credentials are required for Pico W
    // due to it does not have reconnect feature.
#if defined(ARDUINO_RASPBERRY_PI_PICO_W)
    config.wifi.clearAP();
    config.wifi.addAP(WIFI_SSID, WIFI_PASSWORD);
#endif

    /* Assign the callback function for the long running token generation task */
    config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

#if defined(ESP8266)
    // In ESP8266 required for BearSSL rx/tx buffer for large data handle, increase Rx size as needed.
    fbdo.setBSSLBufferSize(2048 /* Rx buffer size in bytes from 512 - 16384 */, 2048 /* Tx buffer size in bytes from 512 - 16384 */);
#endif

    // Limit the size of response payload to be collected in FirebaseData
    fbdo.setResponseSize(2048);

    Firebase.begin(&config, &auth);

    Firebase.reconnectWiFi(true);

    // You can use TCP KeepAlive in FirebaseData object and tracking the server connection status, please read this for detail.
    // https://github.com/mobizt/Firebase-ESP-Client#about-firebasedata-object
    // fbdo.keepAlive(5, 5, 1);
}


void savePayloadToFile(const String& payload) {
  // Open the file in write mode
  File file = SPIFFS.open("/payload.json", "w");
  
  if (file) {
    // Write the payload data to the file
    file.println(payload);
    file.close();
    Serial.println("Payload saved to payload.json");
  } else {
    Serial.println("Failed to open payload.json for writing");
  }
}




void loop()
{

    // Firebase.ready() should be called repeatedly to handle authentication tasks.

    if (Firebase.ready() && (millis() - dataMillis > 60000 || dataMillis == 0))
    {
        dataMillis = millis();

        if (!taskCompleted)
        {
            taskCompleted = true;

            // For the usage of FirebaseJson, see examples/FirebaseJson/BasicUsage/Create_Edit_Parse/Create_Edit_Parse.ino
            FirebaseJson content;

            content.set("fields/Japan/mapValue/fields/time_zone/integerValue", "9");
            content.set("fields/Japan/mapValue/fields/population/integerValue", "125570000");

            content.set("fields/Belgium/mapValue/fields/time_zone/integerValue", "1");
            content.set("fields/Belgium/mapValue/fields/population/integerValue", "11492641");

            content.set("fields/Singapore/mapValue/fields/time_zone/integerValue", "8");
            content.set("fields/Singapore/mapValue/fields/population/integerValue", "5703600");

            // info is the collection id, countries is the document id in collection info.
            String documentPath = "info/countries";

            Serial.print("Create document... ");

            if (Firebase.Firestore.createDocument(&fbdo, FIREBASE_PROJECT_ID, "" /* databaseId can be (default) or empty */, documentPath.c_str(), content.raw()))
                Serial.println(fbdo.payload().c_str());
            else
                Serial.println(fbdo.errorReason());
        }

        String documentPath = "ApprovedUsers/user0";
        String mask = "data";

        // If the document path contains space e.g. "a b c/d e f"
        // It should encode the space as %20 then the path will be "a%20b%20c/d%20e%20f"

        Serial.println("Get a document... ");

        if (Firebase.Firestore.getDocument(&fbdo, FIREBASE_PROJECT_ID, "", documentPath.c_str(), mask.c_str())){
            Serial.println(fbdo.payload());
            DynamicJsonDocument doc(10000);
            // You can use a String as your JSON input.
            // WARNING: the string in the input  will be duplicated in the JsonDocument.
            deserializeJson(doc, fbdo.payload());
            JsonObject obj = doc.as<JsonObject>();
            Serial.println(obj["fields"]["data"]["mapValue"]["fields"]["id"].as<String>());
            
            //Serial.println(doc["feild"]["id"].as<String>());
            /*savePayloadToFile(fbdo.payload());
            File file = SPIFFS.open("/payload.json", "r");
            println()*/
            /*
            String key = "stringValue";
            String extractedValue;
            String jsonString = obj["fields"]["id"].as<String>();
            
            int keyIndex = jsonString.indexOf(key);
            
            if (keyIndex != -1) {
              int startIndex = jsonString.indexOf(":", keyIndex) + 1;
              int endIndex = jsonString.indexOf("}", startIndex);
              
              if (startIndex != -1 && endIndex != -1) {
                extractedValue = jsonString.substring(startIndex, endIndex);
                extractedValue.trim(); // Remove leading/trailing whitespace if any
              }
            }
            
            //Serial.begin(9600);
            Serial.println("Extracted value: " + extractedValue);
            */
        }
        else
            Serial.println(fbdo.errorReason());
    }
}
