# Biometric-Attendance-System Project by: Boran Swaid, Suray Swaid & Foad Hijab.
This project addresses the need for a secure and efficient attendance tracking solution in both online and offline settings.
Our project utilizes fingerprint recognition technology to seamlessly record and manage attendance. It is integrated with Firebase for synchronization and can be controlled through a Telegram bot.

# Details about the project:
This is a Biometric attendance system that lets users register to the system by typing their ID number and scanning their fingerprint, the manager
accepts and rejects users with telegram bot that helps him to manage the pending users list in the Firestore database.
at every attempt of user to enter the system, it saves its entry time to Logs file.
- istallation guide: [instructions.txt](Documentation/instructions.txt)
- user manual: [User_Manual.pdf](Documentation/User_Manual.pdf)

**System Diagram:**

**state machine:**
![image](https://github.com/SuraySweed/Biometric-Attendance-System/assets/75131035/017573bb-e6f7-4c1e-938f-c892c817bc1f)

# libraries:
**Arduino:**
- keyboard: ^1.0.2
- LiquidCrystal: ^1.0.7
- NTPClient: ^3.2.1
- WiFi: ^1.2.7
- Adafruit FingerPrint Sensor Library: ^2.1.2
- Adafruit SSD1306: ^2.5.7
- Adafruit GFX library: ^1.11.7
- Arduino_MKRMEM: ^1.1.0
- ArduinoJson : ^6.21.3
- Firebase Arduino Client Library for ESP8266 and ESP32: ^4.3.19
- Time: ^1.6.1

**Python:**
- telegram: pip install python-telegram-bot
- firebase: pip install firebase-admin

# Folder description:
* ESP32: source code for the esp side (firmware).
* Documentation: wiring diagram + basic operating instructions
* Unit Tests: tests for individual hardware components (input / output devices).
* telegramBot_firebase: source code for telegramBot in python + json file for configration of firstore.
* libraries: arduino/ESP32 libraries used  in this project.

# Project Poster:



IoT Project - 236332, Technion.
