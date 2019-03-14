/*  
  https://gitlab.com/jousis/espresso-scale
*/

#include "settings.h"

#ifdef SERIAL
  #define SERIAL_PRINT(x) Serial.print(x)
  #define SERIAL_PRINTLN(x) Serial.println(x)
  #ifdef DEBUG
    #define DEBUG_PRINT(x) Serial.print(x)
    #define DEBUG_PRINTLN(x) Serial.println(x)
  #else
    #define DEBUG_PRINT(x)
    #define DEBUG_PRINTLN(x)
  #endif
#else
  #define SERIAL_PRINT(x)
  #define SERIAL_PRINTLN(x)
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
#endif

SETTINGS::SETTINGS()
{
  loadSettings();
} 

void SETTINGS::clearEEPROM() {
  preferences.begin("oses", false); //RW
  preferences.clear();
  preferences.end();
}


void SETTINGS::resetSettings() {
  //we only reset the values to default, not applying them.
  //for example, we will reset adcSpeed but not apply it.
  //If you want immediate change, you need to call applySetting(String key,String value, bool saveToEEPROM) for each one
  DEBUG_PRINTLN("resetting all settings to default");
  otaUpgrade = 0;
  snoozeTimeout = DEFAULT_SNOOZE_TIMEOUT;
  lightSleepTimeout = DEFAULT_LIGHT_SLEEP_TIMEOUT;
  deepSleepTimeout = DEFAULT_DEEP_SLEEP_TIMEOUT;
  scaleMode = DEFAULT_SCALE_MODE;
  scaleUnits = DEFAULT_SCALE_UNITS;
  zeroTracking = DEFAULT_ZERO_TRACKING;
  zeroRange = DEFAULT_ZERO_RANGE;
  fakeRange = DEFAULT_FAKE_STABILITY_RANGE;
  stableWeightDiff = DEFAULT_STABLE_WEIGHT_DIFF;
  sensitivity = DEFAULT_SENSITIVITY;
  smoothing = DEFAULT_SMOOTHING;
  adcSpeed = DEFAULT_ADC_SPEED;
  decimalDigits = DEFAULT_DECIMAL_DIGITS;
  bleEnabled = DEFAULT_BLE_ENABLED;
  slBtnPress = DEFAULT_STATUS_LED_BUTTON_PRESS;
  slMaxVIN = DEFAULT_STATUS_LED_MAX_VIN;
  batReadInterval = DEFAULT_BATTERY_LEVEL_READ_INTERVAL;
  calFactorULong = DEFAULT_CAL_FACTOR_ULONG; //scale.setCalFactor(calFactorULong/10.0)
  wifiHostname = DEFAULT_WIFI_HOSTNAME;
  wifiSSID = DEFAULT_WIFI_SSID;
  wifiPassword = DEFAULT_WIFI_PASSWORD;
  readSamples = DEFAULT_READ_SAMPLES;
  saveSettings();
}

bool SETTINGS::loadSettings() {
  // reads variables from EEPROM using preferences library
  DEBUG_PRINTLN("loading settings from EEPROM");
  preferences.begin("oses", true); //read only
  DEBUG_PRINT("Free entries ");DEBUG_PRINTLN(preferences.freeEntries());
  byte major = preferences.getUChar("major",0);
  byte minor = preferences.getUChar("minor",0);
  byte revision = preferences.getUChar("revision",0);
  if (major + minor + revision == 0 ) {
    DEBUG_PRINTLN("initial run, welcome to Open Source Espresso Scale");
    preferences.end();
    return false;
  }
  //Bytes (uint8_t)
  otaUpgrade = preferences.getUChar("otaUpgrade",0);
  snoozeTimeout = preferences.getUChar("snoozeTimeout",DEFAULT_SNOOZE_TIMEOUT);
  lightSleepTimeout = preferences.getUChar("lightSleepTimeout",DEFAULT_LIGHT_SLEEP_TIMEOUT);
  deepSleepTimeout = preferences.getUChar("deepSleepTimeout",DEFAULT_DEEP_SLEEP_TIMEOUT);
  scaleMode = preferences.getUChar("scaleMode",DEFAULT_SCALE_MODE);
  scaleUnits = preferences.getUChar("scaleUnits",DEFAULT_SCALE_UNITS);
  zeroTracking = preferences.getUChar("zeroTracking",DEFAULT_ZERO_TRACKING);
  stableWeightDiff = preferences.getUChar("stableWeightDiff",DEFAULT_STABLE_WEIGHT_DIFF);
  zeroRange = preferences.getUChar("zeroRange",DEFAULT_ZERO_RANGE);
  fakeRange = preferences.getUChar("fakeRange",DEFAULT_FAKE_STABILITY_RANGE);
  sensitivity = preferences.getUChar("sensitivity",DEFAULT_SENSITIVITY);
  smoothing = preferences.getUChar("smoothing",DEFAULT_SMOOTHING);
  adcSpeed = preferences.getUChar("adcSpeed",DEFAULT_ADC_SPEED);
  decimalDigits = preferences.getUChar("decimalDigits",DEFAULT_DECIMAL_DIGITS);
  bleEnabled = preferences.getUChar("bleEnabled",DEFAULT_BLE_ENABLED);
  readSamples = preferences.getUChar("readSamples",DEFAULT_READ_SAMPLES);
  if (readSamples <= 0) { readSamples = 1; }
  slBtnPress = preferences.getUChar("slBtnPress",DEFAULT_STATUS_LED_BUTTON_PRESS);
  slMaxVIN = preferences.getUChar("slMaxVIN",DEFAULT_STATUS_LED_MAX_VIN);
  batReadInterval = preferences.getUChar("batReadInterval",DEFAULT_BATTERY_LEVEL_READ_INTERVAL);

  //Unsigned Longs (uint32_t)
  calFactorULong = preferences.getULong("calFactorULong",DEFAULT_CAL_FACTOR_ULONG);
  

  //Strings  
  wifiHostname = preferences.getString("wifiHostname",DEFAULT_WIFI_HOSTNAME);
  wifiSSID = preferences.getString("wifiSSID",DEFAULT_WIFI_SSID);
  wifiPassword = preferences.getString("wifiPassword",DEFAULT_WIFI_PASSWORD);


  if (preferences.freeEntries() < 100) {
    DEBUG_PRINT("Warning: EEPROM free space is only : ");DEBUG_PRINT(preferences.freeEntries());DEBUG_PRINTLN(" bytes");
  }
  preferences.end();
}


void SETTINGS::saveSettingByte(const char* key, uint8_t value) {
  DEBUG_PRINT("setting ");DEBUG_PRINT(key);
  DEBUG_PRINT(" to ");DEBUG_PRINTLN(value);
  preferences.begin("oses", false); //RW
  preferences.putUChar(key,value);
  preferences.end();  
}
void SETTINGS::saveSettingULong(const char* key, uint32_t value) {
  DEBUG_PRINT("setting ");DEBUG_PRINT(key);
  DEBUG_PRINT(" to ");DEBUG_PRINTLN(value);
  preferences.begin("oses", false); //RW
  preferences.putULong(key,value);
  preferences.end();  
}
void SETTINGS::saveSettingString(const char* key, String value) {
  DEBUG_PRINT("setting ");DEBUG_PRINT(key);
  DEBUG_PRINT(" to ");DEBUG_PRINTLN(value);
  preferences.begin("oses", false); //RW
  preferences.putString(key,value);
  preferences.end();  
}


void SETTINGS::saveSettings() {
  preferences.begin("oses", false); //RW
  
  DEBUG_PRINTLN("Clearing EEPROM");
  preferences.clear();
  
  DEBUG_PRINTLN("Saving all settings to EEPROM");
  preferences.putUChar("major",EEPROM_STRUCT_VERSION_MAJOR);
  preferences.putUChar("minor",EEPROM_STRUCT_VERSION_MINOR);
  preferences.putUChar("revision",EEPROM_STRUCT_VERSION_REVISION);

  preferences.putUChar("otaUpgrade",otaUpgrade);
  preferences.putUChar("snoozeTimeout",snoozeTimeout);
  preferences.putUChar("lightSleepTimeout",lightSleepTimeout);
  preferences.putUChar("deepSleepTimeout",deepSleepTimeout);
  preferences.putUChar("scaleMode",scaleMode);
  preferences.putUChar("scaleUnits",scaleUnits);
  preferences.putUChar("zeroTracking",zeroTracking);
  preferences.putUChar("zeroRange",zeroRange);
  preferences.putUChar("fakeRange",fakeRange);
  preferences.putUChar("stableWeightDiff",stableWeightDiff);
  preferences.putUChar("sensitivity",sensitivity);
  preferences.putUChar("smoothing",smoothing);  
  preferences.putUChar("adcSpeed",adcSpeed);
  preferences.putUChar("decimalDigits",decimalDigits);
  preferences.putUChar("bleEnabled",bleEnabled);
  preferences.putUChar("readSamples",readSamples);
  preferences.putUChar("slBtnPress",slBtnPress);
  preferences.putUChar("slMaxVIN",slMaxVIN);
  preferences.putUChar("batReadInterval",batReadInterval);

  preferences.putULong("calFactorULong",calFactorULong);
  
  preferences.putString("wifiHostname",wifiHostname);
  preferences.putString("wifiSSID",wifiSSID);
  preferences.putString("wifiPassword",wifiPassword);

  preferences.end();
}
