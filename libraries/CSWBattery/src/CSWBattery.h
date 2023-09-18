/**
  ******************************************************************************
  * @file    CSWBattery.h
  * @author  Eugene at sky.community
  * @version V1.0.0
  * @date    12-December-2022
  * @brief   The functionality to work with the battery of the smartwatch based on esp32.
  *
  ******************************************************************************
  */
#ifndef CSWBattery_h
#define CSWBattery_h
#include <stdint.h>
#include <deque>

struct batteryCheck {
  unsigned long time_checked=0;
  float         voltage=-1;
  int           percentage=-1;
  int           section=-1;
};

typedef std::deque<batteryCheck> t_batteryCheck;
typedef void (*VoidFunctionWithNoParameters) (void);

void CSWBattery_tick(void * c);

class CSWBattery {
  public:
    // Constants
    enum data_receiving_type {instantReceive,averageReceive};

    // init
    CSWBattery(int battery_pin=-100, int precision=-100);
    void        setBatteryPin(int battery_pin);
    int         getBatteryPin();
    int         getVoltagePrecision();
    void        setVoltagePrecision(int precision=-1);
    int         getSectionsNum();
    void        setSectionsNum(int sections);
    void        setBatteryStatsReceivingType(data_receiving_type tp=instantReceive);

    // Get battery data
    float       getBatteryVoltage(bool no_update=false, bool get_raw=false, int override_precision=-1, bool check_thoroughly=false, bool use_default_cf=false, int override_num_checks_thoroughly=-1, bool force_instant_value=false, bool force_average_value=false);
    int         getBatteryVoltageSection(bool no_update=false, bool check_thoroughly=false, bool force_instant_check=false);
    int         getBatteryVoltagePercentage(bool no_update=false, bool check_thoroughly=false, bool force_instant_value=false);
    int         getBatteryLowThreshold(void);

    // Different checkers
    bool        checkIfWeAreCharging(bool force_instant_check=false);
    bool        checkIfEmpty(void);
    bool        checkIfLow(void);
    bool        checkBatteryVoltageChanged(int check_type=-1, bool force_instant_check=false);

    unsigned long getLastCheckTime();
    // Setting last-whatever
    void        setLastCheckTime(unsigned long tm=0);
    void        setLastBatteryVoltage(float v);
    void        setLastBatteryVoltageSection(int s);
    void        setLastBatteryVoltagePercentage(int s);
    // Getting last-whatever
    float       getLastBatteryVoltage(void);
    int         getLastBatteryVoltageSection(void);
    int         getLastBatteryVoltagePercentage(void);

    // Calibration
    void        calibrateBattery(int precision=1);
    void        stopCalibration(void);
    void        resetBattery();
    void        setBatteryIsCalibrated(bool cs=true);
    bool        getBatteryIsCalibrated();
    void        setCalibrationIterations(int c);
    int         getCalibrationIterations();
    void        setBatteryCoefficient(float c);
    float       getBatteryCoefficient(bool get_default=false);

    // General
    void        tick(void);
    void        startCollectingData(void);
    void        stopCollectingData(void);
    bool        checkIfCollectingData(void);
    void        flushCollectingDataBuffer(void);
    unsigned long getTimeRecheckS(void);
    void        setBatteryCheckType(int check_type=1);
    int         getBatteryCheckType(void);
    void        setBatteryLowThreshold(int t);

    //events
    void        setHandlerOnBatteryEmpty(VoidFunctionWithNoParameters f);
    void        setHandlerOnBatteryLevelChange(VoidFunctionWithNoParameters f);
    
    // Debug
    void        setDebugLevel(int d=1);
    int         getDebugLevel(void);
    void        setDebug(bool d);
    void        enableDebug(void);
    void        disableDebug(void);

    // Constants
    static const int MIN_TASK_DELAY_S=1;
  protected:
    // Constants
    // Voltages
    const float _fully_charged_voltage=4.2;
    const float _fully_uncharged_voltage=3.7;
    const float _charging_threshold=4.3;
    const float defaultBatteryCf=1.1;
    const int   batteryChecksMinThreshold=3;

    // Different settings connected to iterations and delays
    int         _battery_check_times = 5;
    int         _battery_check_delay_ms = 20;
    int         _battery_recheck_empty_delay_ms = 300;
    int         _sections_num = 5;
    int         batteryCalibrationDelayMS = 750;
    int         _calibrationIterations = 10;
    float       batteryVCalibrationDiffThreshold=0.3;
    float       low_battery_threshold_percent=20;

    // Data to set
    float       _last_battery_voltage=-1;
    int         _last_battery_voltage_section=-1;
    int         _last_battery_voltage_percentage=-1;
    int         _voltage_precision=1;
    int         _battery_pin=-100;
    t_batteryCheck battery_checks;
    bool        _collecting_data_started=false;
    int         _check_type=1;
    bool        _battery_voltage_changed=false;
    bool        _last_charging_status=false;
    
    // Time data
    unsigned long _last_check_tm=0; //TODO: Obsolete? // Last time the battery level was checked 
    unsigned long _time_limit_s=60; // Battery data saved for this amount of seconds
    unsigned long _time_recheck_s=10; // Battery will be rechecked every this amount of seconds

    float       batteryCf=1.1;
    bool        _calibrationStatus=false;
    bool        _stop_calibration = false;

    // General config
    bool        _getAvgData=false;
    bool        _tickBattery=false;

    //events
    VoidFunctionWithNoParameters emptyBatteryHandler=NULL;
    VoidFunctionWithNoParameters changeBatteryLevelHandler=NULL;

    // Debug
    bool        DEBUG = false;
    int         DEBUG_LEVEL = 1;
};
#endif
