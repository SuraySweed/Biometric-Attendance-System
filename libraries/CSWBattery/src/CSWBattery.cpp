/**
  ******************************************************************************
  * @file    CSWBattery.cpp
  * @author  Eugene at sky.community
  * @version V1.0.0
  * @date    12-December-2022
  * @brief   The functionality to work with the battery of the smartwatch based on esp32.
  *
  ******************************************************************************
  */
#define uS_TO_S_FACTOR 1000000ULL
#define mS_TO_S_FACTOR 1000

#include <Arduino.h>
#include "CSWBattery.h"
typedef void (*VoidFunctionWithNoParameters) (void);

CSWBattery::CSWBattery(int battery_pin, int precision) {
  if(DEBUG && (DEBUG_LEVEL >=1) && Serial) {
    Serial.println("[CSWBattery] Initializing battery.");
  }
  if(DEBUG && (DEBUG_LEVEL >=10) && Serial) {
    Serial.print("[CSWBattery] Battery params: ");
    Serial.print("  - battery pin: ");
    Serial.println(battery_pin);
    Serial.print("  - precision: ");
    Serial.println(precision);
  }
  if(battery_pin != -100) _battery_pin = battery_pin;
  if(precision != -100) _voltage_precision = precision;
}

int CSWBattery::getSectionsNum() {
  return _sections_num;
}
void CSWBattery::setSectionsNum(int sections) {
  if(DEBUG && (DEBUG_LEVEL >=1) && Serial) {
    Serial.print("[CSWBattery] Setting sections num to: ");
    Serial.println(sections);
  }
  _sections_num = sections;
}

void CSWBattery::setBatteryPin(int battery_pin) {
  if(DEBUG && (DEBUG_LEVEL >=1) && Serial) {
    Serial.print("[CSWBattery] Setting battery pin to: ");
    Serial.println(battery_pin);
  }
  _battery_pin = battery_pin;
}

int CSWBattery::getBatteryPin() {
  if(DEBUG && (DEBUG_LEVEL >=99) && Serial) {
    Serial.print("[CSWBattery] Getting battery pin: ");
    Serial.println(_battery_pin);
  }
  return _battery_pin;
}

int CSWBattery::getVoltagePrecision() {
  if(DEBUG && (DEBUG_LEVEL >=99) && Serial) {
    Serial.print("[CSWBattery] Getting voltage precision: ");
    Serial.println(_voltage_precision);
  }
  return _voltage_precision;
}

void CSWBattery::setVoltagePrecision(int precision) {
  if(DEBUG && (DEBUG_LEVEL >=1) && Serial) {
    Serial.print("[CSWBattery] Setting voltage precision to: ");
    Serial.println(precision);
  }
  _voltage_precision = precision;
}

void CSWBattery::setBatteryCoefficient(float c) {
  if(DEBUG && (DEBUG_LEVEL >=1) && Serial) {
    Serial.print("[CSWBattery] Setting battery coefficient to: ");
    Serial.println(c);
  }
  batteryCf=c;
}

float CSWBattery::getBatteryCoefficient(bool get_default) {
  if(DEBUG && (DEBUG_LEVEL >=99) && Serial) {
    Serial.print("[CSWBattery] Getting battery coefficient");
    Serial.print(get_default ? " (default one)" : "");
    Serial.print(": ");
    Serial.println(get_default ? defaultBatteryCf : batteryCf);
  }
  return get_default ? defaultBatteryCf : batteryCf;
}

bool CSWBattery::checkBatteryVoltageChanged(int check_type, bool force_instant_check) {
  if(check_type == -1) check_type = _check_type;
  this->setLastCheckTime();
  if((!force_instant_check) && _getAvgData) {
    bool __rres = _battery_voltage_changed;
    _battery_voltage_changed = false;
    return __rres;
  }
  bool res = true;
  int _lbvi;
  int _bvi;
  float _lbv;
  float _bv;
  if(DEBUG && (DEBUG_LEVEL >=99) && Serial) {
    Serial.print("[CSWBattery] Checking if battery voltage has changed. Type of check is: ");
  }
  // Firstly let's check for charger attached.
  // It'll be for all checks the "true" result;
  bool currently_charging = this->checkIfWeAreCharging(true);
  switch(check_type) {
    case 1:
      //sections
      if(DEBUG && (DEBUG_LEVEL >=99) && Serial) {
        Serial.print("sections: ");
      }
      _lbvi = _last_battery_voltage_section;
      _bvi = this->getBatteryVoltageSection(false,false,force_instant_check);
      if (_lbvi!=_bvi) {
        for(int i=0;i<_battery_check_times;i++) {
          _bvi = this->getBatteryVoltageSection(false,false,force_instant_check);
          if (_lbvi == _bvi) {
            if(DEBUG && (DEBUG_LEVEL >=99) && Serial) {
              Serial.print("Got fluke.");
            }
            res = false; //fluke
          }
          delay(_battery_check_delay_ms);
        }
        if(DEBUG && (DEBUG_LEVEL >=99) && Serial) {
          Serial.println(res ? "Changed." : "Not changed.");
        }
        _battery_voltage_changed = res;
        return res;
      } else {
        if(
          (currently_charging && (_lbvi != -1))
          ||
          ((!currently_charging) && (_lbvi == -1))) {
          if(this->checkIfCollectingData()) this->flushCollectingDataBuffer();
          if(DEBUG && (DEBUG_LEVEL >=99) && Serial) {
            Serial.println("Changed.");
          }
          _battery_voltage_changed = true;
          return true;
        } else {
          if(DEBUG && (DEBUG_LEVEL >=99) && Serial) {
            Serial.println("Not changed.");
          }
          _battery_voltage_changed = false;
          return false;
        }
      }
    break;
    case 2:
      //percentage
      if(DEBUG && (DEBUG_LEVEL >=99) && Serial) {
        Serial.print("precentage: ");
      }
      _lbvi = _last_battery_voltage_percentage;
      _bvi = this->getBatteryVoltagePercentage(false,false,force_instant_check);
      if (_lbvi!=_bvi) {
        for(int i=0;i<_battery_check_times;i++) {
          _bvi = this->getBatteryVoltagePercentage(false,false,force_instant_check);
          if (_lbvi == _bvi) {
            if(DEBUG && (DEBUG_LEVEL >=99) && Serial) {
              Serial.print("Got fluke.");
            }
            res = false; //fluke
          }
          delay(_battery_check_delay_ms);
        }
        if(DEBUG && (DEBUG_LEVEL >=99) && Serial) {
          Serial.println(res ? "Changed." : "Not changed.");
        }
        _battery_voltage_changed = res;
        return res;
      } else {
        if(
          (currently_charging && (_lbvi != -1))
          ||
          ((!currently_charging) && (_lbvi == -1))) {
          if(this->checkIfCollectingData()) this->flushCollectingDataBuffer();
          if(DEBUG && (DEBUG_LEVEL >=99) && Serial) {
            Serial.println("Changed.");
          }
          _battery_voltage_changed = true;
          return true;
        } else {
          if(DEBUG && (DEBUG_LEVEL >=99) && Serial) {
            Serial.println("Not changed.");
          }
          _battery_voltage_changed = false;
          return false;
        }
      }
      //return (_last_battery_voltage_percentage!=this->getBatteryVoltagePercentage());
    case 3:
      //voltage
    default:
      if(DEBUG && (DEBUG_LEVEL >=99) && Serial) {
        Serial.print("voltage: ");
      }
      _lbv = _last_battery_voltage;
      _bv = this->getBatteryVoltage(false,false,-1,false,false,-1,force_instant_check);
      if (_lbv!=_bv) {
        for(int i=0;i<_battery_check_times;i++) {
          _bv = this->getBatteryVoltage(false,false,-1,false,false,-1,force_instant_check);
          if (_lbv == _bv) {
            if(DEBUG && (DEBUG_LEVEL >=99) && Serial) {
              Serial.print("Got fluke.");
            }
            res = false; //fluke
          }
          delay(_battery_check_delay_ms);
        }
        if(DEBUG && (DEBUG_LEVEL >=99) && Serial) {
          Serial.println(res ? "Changed." : "Not changed.");
        }
        _battery_voltage_changed = res;
        return res;
      } else {
        if(
          (currently_charging && (_lbv < _charging_threshold))
          ||
          ((!currently_charging) && (_lbv >= _charging_threshold))) {
          if(DEBUG && (DEBUG_LEVEL >=99) && Serial) {
            Serial.println("Changed.");
          }
          if(this->checkIfCollectingData()) this->flushCollectingDataBuffer();
          _battery_voltage_changed = true;
          return true;
        }
        else {
          if(DEBUG && (DEBUG_LEVEL >=99) && Serial) {
            Serial.println("Not changed.");
          }
          _battery_voltage_changed = false;
          return false;
        }
      }
  }
  _battery_voltage_changed = false;
  return __null;
}

//TODO: Obsolete?
unsigned long CSWBattery::getLastCheckTime() {
  if(DEBUG && (DEBUG_LEVEL >=99) && Serial) {
    Serial.print("[CSWBattery] Getting last check time: ");
    if(_last_check_tm > 0) Serial.println(_last_check_tm); else Serial.println("NULL");
  }
  return (_last_check_tm > 0) ? _last_check_tm : __null;
}
//TODO: Obsolete?
void CSWBattery::setLastCheckTime(unsigned long tm) {
  if(DEBUG && (DEBUG_LEVEL >=10) && Serial) {
    Serial.print("[CSWBattery] Setting last check time to: ");
    Serial.println(tm);
  }
  _last_check_tm = (tm == 0) ? millis() : tm;
}

float CSWBattery::getLastBatteryVoltage() {
  if(DEBUG && (DEBUG_LEVEL >=99) && Serial) {
    Serial.print("[CSWBattery] Getting last battery voltage: ");
    Serial.println(_last_battery_voltage);
  }
  return _last_battery_voltage;
}
void CSWBattery::setLastBatteryVoltage(float v) {
  if(DEBUG && (DEBUG_LEVEL >=10) && Serial) {
    Serial.print("[CSWBattery] Setting battery last voltage to: ");
    Serial.println(v);
  }
  _last_battery_voltage = v;
}

/// @brief Set the receiving data type - if it is the "average" data for some period of time or the instant one
/// @param tp (default) - instant; - average;
void CSWBattery::setBatteryStatsReceivingType(data_receiving_type tp) {
  if(tp == averageReceive) _getAvgData = true; else _getAvgData = false;
  if(_getAvgData && (!_collecting_data_started)) this->startCollectingData();
}

float CSWBattery::getBatteryVoltage(bool no_update, bool get_raw, int override_precision, bool check_thoroughly, bool use_default_cf, int override_num_checks_thoroughly, bool force_instant_value, bool force_average_value) {
  if(DEBUG && (DEBUG_LEVEL >=99) && Serial) {
    Serial.println("[CSWBattery] Getting battery voltage with params: ");
    if(no_update) Serial.println("- no update;");
    if(get_raw) {
      Serial.print("  - get raw data (without coefficient - current value is ");
      Serial.print(this->getBatteryCoefficient());
      Serial.println(");");
    }
    if(override_precision != -1) {
      Serial.print("  - override battery precision to: ");
      Serial.print(override_precision);
      Serial.println(");");
    }
    if(check_thoroughly) {
      Serial.print("  - check thoroughly (");
      Serial.print((override_num_checks_thoroughly != -1) ? _battery_check_times : override_num_checks_thoroughly);
      Serial.print(" times with a delay ");
      Serial.print(_battery_check_delay_ms);
      Serial.println(" ms and afterwards get the average value);");
    }
    if(use_default_cf) {
      Serial.print("  - using default coefficient: ");
      Serial.print(this->getBatteryCoefficient(true));
      Serial.println(");");
    }
    if(override_num_checks_thoroughly != -1) {
      Serial.print("  - override number of checks for thoroughly checking to: ");
      Serial.print(override_num_checks_thoroughly);
      Serial.println(");");
    }
  }
  float __cf = this->getBatteryCoefficient(false);
  float __cfD = this->getBatteryCoefficient(true);
  float __last_battery_voltage=0;
  if(_getAvgData && force_average_value && (battery_checks.size() < batteryChecksMinThreshold)) {
    return -100;
  }
  if((_getAvgData) && (
    (!force_instant_value)
    &&
    (override_precision == -1) // we have collected data with just one precision only.
    &&
    (battery_checks.size() >= batteryChecksMinThreshold) // can't return average data without sufficient amount of... data
    &&
    (__cf != 0) //if that was broken
  )) {
    if(DEBUG && (DEBUG_LEVEL >=99) && Serial) {
      Serial.println("[CSWBattery] Processing average data.");
      Serial.print("[CSWBattery] Size of stack: ");
      Serial.println(battery_checks.size());
    }
    float __res = -1;
    float __cf = this->getBatteryCoefficient(use_default_cf);
    float sum_of_elems=0;
    for (auto& __bc : battery_checks) sum_of_elems += __bc.voltage;
    __res = sum_of_elems / battery_checks.size();
    __res = (_voltage_precision >= 2) ? 
      round(__res * pow(10.0,_voltage_precision)) / pow(10.0,_voltage_precision)
      :
      ( (_voltage_precision == 1) ? round(__res * 10.0) / 10.0 : floor(__res) );
    if(use_default_cf && (!get_raw)) __res = __res * __cfD /__cf;
    if(__res < 0) __res = -1;
    __last_battery_voltage = __res;
    if(get_raw && (__res >=0)) __res = __res / __cf; //default CF in here doesn't make sense as the data was collected using non-default one
    else if(get_raw) __res = -1;
    if(!no_update) _last_battery_voltage = __last_battery_voltage;
    return __res;
  }
  if((_getAvgData) && (battery_checks.size() < batteryChecksMinThreshold) && (!force_instant_value)) {
    if(DEBUG && (DEBUG_LEVEL >=99) && Serial) {
      Serial.println("[CSWBattery] Due to lack of collected data we'll override check_thoroughly value to true");
      Serial.println("[CSWBattery] and just collect the current data - thoroughly.");
    }
    check_thoroughly = true;
  }
  int __voltage_precision = (override_precision != -1) ? override_precision : _voltage_precision;
  float __batteryCf = this->getBatteryCoefficient(use_default_cf);
  if(check_thoroughly) {
    int __battery_check_times = (override_num_checks_thoroughly == -1) ? _battery_check_times : override_num_checks_thoroughly;
    for(int i=0;i<__battery_check_times;i++) {
      int _battery_pin_current_value = analogRead(_battery_pin);
      __last_battery_voltage += (__voltage_precision >= 2) ?
        round(((float)(_battery_pin_current_value) / 4095*2*3.3*(get_raw ? 1 : __batteryCf))* pow(10.0,__voltage_precision)) / pow(10.0,__voltage_precision)
        :
        (
          (__voltage_precision == 1) ?
          round(((float)(_battery_pin_current_value) / 4095*2*3.3*(get_raw ? 1 : __batteryCf))* 10.0) / 10.0
          :
          floor(((float)(_battery_pin_current_value) / 4095*2*3.3*(get_raw ? 1 : batteryCf)))
        );
        delay(_battery_check_delay_ms);
    }
    __last_battery_voltage = __last_battery_voltage/__battery_check_times;
    __last_battery_voltage = (__voltage_precision >= 2) ? round(__last_battery_voltage * pow(10.0,__voltage_precision)) / pow(10.0,__voltage_precision)
        :
        (
          (__voltage_precision == 1) ?
          round(__last_battery_voltage * 10.0) / 10.0
          :
          floor(__last_battery_voltage)
        );
  } else {
    int _battery_pin_current_value = analogRead(_battery_pin);
    __last_battery_voltage = (__voltage_precision >= 2) ?
      round(((float)(_battery_pin_current_value) / 4095*2*3.3*(get_raw ? 1 : __batteryCf))* pow(10.0,__voltage_precision)) / pow(10.0,__voltage_precision)
      :
      (
        (__voltage_precision == 1) ?
        round(((float)(_battery_pin_current_value) / 4095*2*3.3*(get_raw ? 1 : __batteryCf))* 10.0) / 10.0
        :
        floor(((float)(_battery_pin_current_value) / 4095*2*3.3*(get_raw ? 1 : __batteryCf)))
      );
  }
  if(!no_update) _last_battery_voltage = __last_battery_voltage;
  return __last_battery_voltage;
}

int CSWBattery::getBatteryVoltageSection(bool no_update, bool check_thoroughly, bool force_instant_check) {
  if(DEBUG && (DEBUG_LEVEL >=99) && Serial) {
    Serial.print("[CSWBattery] Getting battery voltage section with params: ");
    Serial.println(no_update ? "  - without update;" : "");
    Serial.println(check_thoroughly ? " - check thoroughly;" : "");
  }
  float v = this->getBatteryVoltage(false, false, -1, check_thoroughly,false,-1,force_instant_check);
  int voltage_section = -1;
  if (v >= _charging_threshold) voltage_section = -1; else {
    voltage_section = ceil((min(v,_fully_charged_voltage)-_fully_uncharged_voltage) * _sections_num/(_fully_charged_voltage-_fully_uncharged_voltage));
  }
  if(!no_update) _last_battery_voltage_section = voltage_section;
  return voltage_section;
}

int CSWBattery::getBatteryVoltagePercentage(bool no_update, bool check_thoroughly, bool force_instant_value) {
  if(DEBUG && (DEBUG_LEVEL >=99) && Serial) {
    Serial.print("[CSWBattery] Getting battery voltage percentage with params: ");
    Serial.println(no_update ? "  - without update;" : "");
    Serial.println(check_thoroughly ? " - check thoroughly;" : "");
  }
  float v = this->getBatteryVoltage(false, false, -1, check_thoroughly,false,-1,force_instant_value);
  int voltage_p = -1;
  if (v >= _charging_threshold) voltage_p = -1; else {
    voltage_p = round((min(v,_fully_charged_voltage)-_fully_uncharged_voltage)*100/(_fully_charged_voltage-_fully_uncharged_voltage));
  }
  if(!no_update) _last_battery_voltage_percentage = voltage_p;
  return voltage_p;
}

void CSWBattery::setLastBatteryVoltageSection(int s) {
  if(DEBUG && (DEBUG_LEVEL >=10) && Serial) {
    Serial.print("[CSWBattery] Setting last battery voltage section to: ");
    Serial.println(s);
  }
  _last_battery_voltage_section = s;
}

int CSWBattery::getLastBatteryVoltageSection() {
  if(DEBUG && (DEBUG_LEVEL >=99) && Serial) {
    Serial.print("[CSWBattery] Getting battery last voltage section: ");
    Serial.println(_last_battery_voltage_section);
  }
  return _last_battery_voltage_section;
}

void CSWBattery::setLastBatteryVoltagePercentage(int s) {
  if(DEBUG && (DEBUG_LEVEL >=10) && Serial) {
    Serial.print("[CSWBattery] Setting last battery voltage percentage time to: ");
    Serial.println(s);
  }
  _last_battery_voltage_percentage = s;
}

int CSWBattery::getLastBatteryVoltagePercentage() {
  if(DEBUG && (DEBUG_LEVEL >=99) && Serial) {
    Serial.print("[CSWBattery] Getting battery last voltage percentage: ");
    Serial.println(_last_battery_voltage_percentage);
  }
  return _last_battery_voltage_percentage;
}

bool CSWBattery::checkIfWeAreCharging(bool force_instant_check) {
  bool res = (_getAvgData && (!force_instant_check)) ?
    (this->getLastBatteryVoltage() >= _charging_threshold)
     :
    (this->getBatteryVoltage(false,false,-1,false,false,-1,force_instant_check) >= _charging_threshold);
  if(DEBUG && (DEBUG_LEVEL >=99) && Serial) {
    Serial.print("[CSWBattery] Checking if we are charging: ");
    Serial.println(res ? "yes." : "no.");
  }
  return res;
}

void CSWBattery::setBatteryIsCalibrated(bool cs) {
  if(DEBUG && (DEBUG_LEVEL >=10) && Serial) {
    Serial.print("[CSWBattery] Setting battery calibration to: ");
    Serial.println(cs ? "calibrated." : "not calibrated.");
  }
  _calibrationStatus = cs;
}
bool CSWBattery::getBatteryIsCalibrated() {
  if(DEBUG && (DEBUG_LEVEL >=99) && Serial) {
    Serial.print("[CSWBattery] Checking if battery is calibrated: ");
    Serial.println(_calibrationStatus ? "yes." : "no.");
  }
  return _calibrationStatus;
}

int CSWBattery::getBatteryLowThreshold(void) {
  return low_battery_threshold_percent;
}

void CSWBattery::setBatteryLowThreshold(int t) {
  low_battery_threshold_percent = t;
}

bool CSWBattery::checkIfLow() {
  int b_p = -1;
  b_p = this->getLastBatteryVoltagePercentage();
  return ((b_p != -1) && (b_p <= low_battery_threshold_percent));
}

bool CSWBattery::checkIfEmpty() {
  float __v;
  bool res = false;
  if(_getAvgData) {
    __v = this->getBatteryVoltage(false, false, -1, true, false, -1, false, true);
    if(__v == -100) {
      if(DEBUG && (DEBUG_LEVEL >=99) && Serial) {
        Serial.println("[CSWBattery] Checking if battery is empty: ...");
        Serial.println("[CSWBattery] No sufficient amount of data was captured so saying 'no'.");
      }
      return false; // we want average data to be really sure if it's not a fluke
    } // (__v == -100)
  } else { //!_getAvgData
    __v = this->getBatteryVoltage(false, false, -1, true);
    res = ((__v <= _fully_uncharged_voltage) && (__v >=0));
    if(res) { //res && !_getAvgData => check just once more - just in case, with a small delay
      delay(_battery_recheck_empty_delay_ms);
      __v = this->getBatteryVoltage(false, false, -1, true);
    }
  }
  res = ((__v <= _fully_uncharged_voltage) && (__v >=0));
  if(DEBUG && (DEBUG_LEVEL >=99) && Serial) {
    Serial.print("[CSWBattery] Checking if battery is empty: ");
    Serial.println(res ? "yes." : "no.");
    Serial.print("[CSWBattery] Voltage captured: ");
    Serial.print(__v);
    Serial.println("V.");
  }
  return res;
}

void CSWBattery::resetBattery() {
  if(DEBUG && (DEBUG_LEVEL >=1) && Serial) {
    Serial.print("[CSWBattery] Resetting battery coefficient. New value is: ");
    Serial.println(defaultBatteryCf);
  }
  batteryCf = defaultBatteryCf;
  this->setBatteryIsCalibrated(false);
}

void CSWBattery::stopCalibration(void) {
  _stop_calibration = true;
}

void CSWBattery::calibrateBattery(int precision) {
  if(DEBUG && (DEBUG_LEVEL >=1) && Serial) {
    Serial.print("[CSWBattery] Calibrating battery. Current coefficient value is: ");
    Serial.println(this->getBatteryCoefficient());
  }
  _stop_calibration = false;
  float batteryV1=getBatteryVoltage(true, false, -1, true, true, -1, true);
  float batteryV2=batteryV1;
  // we olny care about the difference between the values - the first
  // value (on charger) is none of our interest
  while (abs(batteryV1 - batteryV2) <= batteryVCalibrationDiffThreshold) {
    //waiting for the unplug of the charger
    batteryV2 = getBatteryVoltage(true, false, -1, true, true, -1, true);
    delay(100);
    if(_stop_calibration) return;
  }
  // because when the board is unplugged the battery voltage can go
  // dramatically down and then back to normal
  delay(batteryCalibrationDelayMS);
  //now batteryV2 HAS to be equal to _fully_charged_voltage;
  batteryV1 = getBatteryVoltage(true, true, precision, true, false, _calibrationIterations, true);
  batteryCf = _fully_charged_voltage / batteryV1;
  if(DEBUG && (DEBUG_LEVEL >=1) && Serial) {
    Serial.print("[CSWBattery] New coefficient value is: ");
    Serial.println(this->getBatteryCoefficient());
  }
  this->setBatteryIsCalibrated(true);
}

void CSWBattery::setCalibrationIterations(int c) {
  if(DEBUG && (DEBUG_LEVEL >=1) && Serial) {
    Serial.print("[CSWBattery] Setting battery calibration interations to: ");
    Serial.println(c);
  }
  _calibrationIterations = c;
}
int CSWBattery::getCalibrationIterations() {
  if(DEBUG && (DEBUG_LEVEL >=99) && Serial) {
    Serial.print("[CSWBattery] Getting calibration iterations: ");
    Serial.println(_calibrationIterations);
  }
  return _calibrationIterations;
}

void CSWBattery::setBatteryCheckType(int check_type) {
  _check_type = check_type;
}
int CSWBattery::getBatteryCheckType(void) {
  return _check_type;
}

void CSWBattery::tick(void) {
  batteryCheck __firstElem;
  unsigned long __current_time = millis();
  if(battery_checks.size() > 0) {
    __firstElem = battery_checks.front();
    if(__current_time -  __firstElem.time_checked < _time_recheck_s*mS_TO_S_FACTOR) return;
  }
  batteryCheck __batCheck;
  __batCheck.time_checked = __current_time;
  __batCheck.percentage = this->getBatteryVoltagePercentage(false, true, true);
  __batCheck.section = this->getBatteryVoltageSection(false, true, true);
  __batCheck.voltage = this->getLastBatteryVoltage();
  __firstElem = (battery_checks.size() > 0) ? battery_checks.front() : __batCheck;
  battery_checks.push_back(__batCheck);
  // FIFO
  while((battery_checks.size() > 0) && (__current_time - __firstElem.time_checked > _time_limit_s*mS_TO_S_FACTOR)) {
    battery_checks.pop_front();
    if(battery_checks.size() > 0) __firstElem = battery_checks.front(); else break;
  }
  bool __last_charging_status = _last_charging_status;
  _last_charging_status = this->checkIfWeAreCharging(true);
  bool __chargingStatusChanged = (__last_charging_status != _last_charging_status);
  if (__chargingStatusChanged) {
    if(DEBUG && (DEBUG_LEVEL >=99) && Serial) {
      Serial.println("[CSWBattery] Flushing buffer and setting voltage status as changed because the charging status has changed.");
    }
    this->flushCollectingDataBuffer();
    _battery_voltage_changed=true;
  }
  bool bvChanged = this->checkBatteryVoltageChanged(2,true);
  if (__chargingStatusChanged || bvChanged) {
    if(DEBUG && (DEBUG_LEVEL >=99) && Serial) {
      Serial.println("[CSWBattery] Setting to True the voltage changed status.");
    }
    _battery_voltage_changed=true;
  }
  if(_battery_voltage_changed && (changeBatteryLevelHandler != NULL)) { //this->checkBatteryVoltageChanged()
    if(DEBUG && (DEBUG_LEVEL >=99) && Serial) {
      Serial.println("[CSWBattery] Calling changeBatteryLevelHandler!");
    }
    changeBatteryLevelHandler();
  }
  if(this->checkIfEmpty() && (emptyBatteryHandler != NULL)) {
    if(DEBUG && (DEBUG_LEVEL >=99) && Serial) {
      Serial.println("[CSWBattery] Calling emptyBatteryHandler!");
    }
    emptyBatteryHandler();
  }

  if(DEBUG && (DEBUG_LEVEL >=99) && Serial) {
    unsigned long __current_time_end = millis();
    Serial.print("[CSWBattery] Tick lasted ");
    Serial.print(__current_time_end-__current_time);
    Serial.println(" ms.");
  }
}
void CSWBattery::startCollectingData(void) {
  if(DEBUG && (DEBUG_LEVEL >=1) && Serial) Serial.println("[CSWBattery] Starting collecting data.");
  _collecting_data_started = true;
  xTaskCreate(
    CSWBattery_tick,    // Function that should be called
    "CSWBattery Tick",   // Name of the task (for debugging)
    4096,            // Stack size (bytes)
    this,            // Parameter to pass
    1,               // Task priority
    NULL             // Task handle
  );
}

void CSWBattery::stopCollectingData(void) {
  _collecting_data_started = false;
}

bool CSWBattery::checkIfCollectingData(void) {
  return _collecting_data_started;
}
void CSWBattery::flushCollectingDataBuffer(void) {
  battery_checks.clear();
}

void CSWBattery::setDebugLevel(int d) {
  if(DEBUG && Serial) {
    Serial.print("[CSWBattery] Setting debug level to: ");
    Serial.println(d);
  }
  if(d >= -1) DEBUG_LEVEL = d;
}

int CSWBattery::getDebugLevel(void) {
  if(DEBUG && (DEBUG_LEVEL >=99) && Serial) {
    Serial.print("[CSWBattery] Getting debug level: ");
    Serial.println(DEBUG_LEVEL);
  }
  return DEBUG_LEVEL;
}
void CSWBattery::setDebug(bool d) {
  if(DEBUG && Serial) {
    Serial.print("[CSWBattery] Setting debug to: ");
    Serial.println(d ? "on." : "off");
  }
  DEBUG = d;
}
void CSWBattery::enableDebug(void) {
  if(DEBUG && Serial) {
    Serial.print("[CSWBattery] Setting debug to: on.");
  }
  DEBUG = true;
}
void CSWBattery::disableDebug(void) {
  if(DEBUG && Serial) {
    Serial.print("[CSWBattery] Setting debug to: off.");
  }
  DEBUG = false;
}
unsigned long CSWBattery::getTimeRecheckS(void) {
  return _time_recheck_s;
}

void CSWBattery::setHandlerOnBatteryEmpty(VoidFunctionWithNoParameters f) {
  emptyBatteryHandler = f;
}

void CSWBattery::setHandlerOnBatteryLevelChange(VoidFunctionWithNoParameters f) {
  changeBatteryLevelHandler = f;
}

void CSWBattery_tick(void * c) {
  CSWBattery * __battery = static_cast<CSWBattery *>(c);
  for(;;) {
    if(!__battery->checkIfCollectingData()) break;
    __battery->tick();
    unsigned long __min_t = CSWBattery::MIN_TASK_DELAY_S;
    vTaskDelay(max(__battery->getTimeRecheckS(),__min_t) * mS_TO_S_FACTOR / portTICK_PERIOD_MS);
  }
  __battery->stopCollectingData();
  vTaskDelete(NULL);
}