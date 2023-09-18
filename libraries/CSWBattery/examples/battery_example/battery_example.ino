#include <CSWBattery.h>


////////////////////////////////////////////////////////////////////////
//Constants
////////////////////////////////////////////////////////////////////////

#define PWR_EN 5

//battery params
//battery display mode possible values
#define VBAT_DISPLAY_ICON 1
#define VBAT_DISPLAY_PERCENTAGE 2
#define VBAT_DISPLAY_VOLTAGE 3

//battery config
#define VBAT_SECTIONS_NUM 5
#define VBAT_VOLTAGE_PRECISION 1
#define VBAT_PIN 34


#define BTR_TIMEOUT 5000


////////////////////////////////////////////////////////////////////////
//EOF Constants
///////////////////////////////////////////////////////////////////////

int batteryDisplayMode = VBAT_DISPLAY_ICON;

//init our battery
CSWBattery BatteryA(VBAT_PIN, VBAT_VOLTAGE_PRECISION);

void showBatteryCallback() {
  switch (batteryDisplayMode) {
    case VBAT_DISPLAY_PERCENTAGE:
      if (BatteryA.checkIfWeAreCharging() || (BatteryA.getLastBatteryVoltagePercentage() == -1)) {
        Serial.println("The battery is charging.");
      } else {
        Serial.print("Battery level: ");
        Serial.print(BatteryA.getBatteryVoltagePercentage());
        Serial.print("%");
      }
      break;
    case VBAT_DISPLAY_VOLTAGE:
      Serial.print("Battery level: ");
      Serial.printf("%.*f", VBAT_VOLTAGE_PRECISION, BatteryA.getLastBatteryVoltage());
      Serial.print("V");
      break;
    case VBAT_DISPLAY_ICON:
      //draw icon. Required graphics, which is not covered in this example.
    default:
      //VBAT_DISPLAY_ICON
      break;
  }
}

void showBattery(bool force_show = false) {
  if (millis() - BatteryA.getLastCheckTime() <= BTR_TIMEOUT) return;
  BatteryA.setLastCheckTime();
  Serial.println("Battery probing: Voltage = "); Serial.print(BatteryA.getLastBatteryVoltage()); Serial.print(" Volts; ");
  if (force_show || BatteryA.checkBatteryVoltageChanged(batteryDisplayMode)) {
    Serial.println("Retrieving battery status...");
    showBatteryCallback();
  }
}

void setup() {
  Serial.begin(115200);
  delay(10);
  Serial.println("Battery test");
  pinMode(PWR_EN,OUTPUT);

  digitalWrite(PWR_EN,HIGH);
  
  BatteryA.setSectionsNum(VBAT_SECTIONS_NUM);
  BatteryA.setBatteryCoefficient(1.1);
  BatteryA.setLastCheckTime();
  
  #if defined(DEBUG) && DEBUG>=1
  Serial.println("Setup done.");
  #endif
}
void loop() {
  showBattery();
}
