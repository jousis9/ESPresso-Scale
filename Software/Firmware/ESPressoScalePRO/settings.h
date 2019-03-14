/*  
  ESPresso Scale
  Created by John Sartzetakis, Jan 2019
  Released into the public domain.
  https://gitlab.com/jousis/espresso-scale
*/

#ifndef SETTINGS_h
#define SETTINGS_h

#include <Preferences.h>

class SETTINGS
{

  public:
    SETTINGS();
    byte otaUpgrade = 0; //OTA is enabled by long touching T8 & T9 during startup
    String wifiHostname = ""; //check out the default values at resetSettings()
    String wifiSSID = "";
    String wifiPassword = "";
    
    byte bleEnabled = DEFAULT_BLE_ENABLED;

    byte snoozeTimeout = DEFAULT_SNOOZE_TIMEOUT;
    byte lightSleepTimeout = DEFAULT_LIGHT_SLEEP_TIMEOUT;
    byte deepSleepTimeout = DEFAULT_DEEP_SLEEP_TIMEOUT;
    byte scaleMode = DEFAULT_SCALE_MODE;
    byte scaleUnits = DEFAULT_SCALE_UNITS;
    byte zeroTracking = DEFAULT_ZERO_TRACKING;
    byte stableWeightDiff = DEFAULT_STABLE_WEIGHT_DIFF;
    byte zeroRange = DEFAULT_ZERO_RANGE;
    byte fakeRange = DEFAULT_FAKE_STABILITY_RANGE;
    byte sensitivity = DEFAULT_SENSITIVITY;
    byte smoothing = DEFAULT_SMOOTHING;
    byte adcSpeed = DEFAULT_ADC_SPEED;
    byte decimalDigits = DEFAULT_DECIMAL_DIGITS;
    byte readSamples = DEFAULT_READ_SAMPLES;
    byte slBtnPress = DEFAULT_STATUS_LED_BUTTON_PRESS;
    byte slMaxVIN = DEFAULT_STATUS_LED_MAX_VIN;
    byte batReadInterval = DEFAULT_BATTERY_LEVEL_READ_INTERVAL;
    uint32_t calFactorULong = DEFAULT_CAL_FACTOR_ULONG;
    void clearEEPROM();
    void resetSettings();
    bool loadSettings();
    void saveSettingByte(const char* key, uint8_t value);
    void saveSettingULong(const char* key, uint32_t value);
    void saveSettingString(const char* key, String value);
    void saveSettings();
  
  protected:
    Preferences preferences;
    //If you change anything in the EEPROM save function change the revision/...
    //and alter the reading function accordingly (optional)
    const byte EEPROM_STRUCT_VERSION_MAJOR = 0;
    const byte EEPROM_STRUCT_VERSION_MINOR = 0;
    const byte EEPROM_STRUCT_VERSION_REVISION = 1;
    
    ///// DEFAULT SETTINGS /////
    const byte DEFAULT_SNOOZE_TIMEOUT = 0; //3 = 30seconds, 0=disabled, in 10*seconds, dims the display, inserts 250ms delay in the loop=> approx 4 ADC reads per second
    const byte DEFAULT_LIGHT_SLEEP_TIMEOUT = 0; //6 = 1min, 0=disabled, in 10*seconds, shuts down ADC using its power pin (not the LDO) and dims the display
    const byte DEFAULT_DEEP_SLEEP_TIMEOUT = 0; //12 = 2mins, 0=disabled, in 10*seconds, shuts down ADC LDO,display and puts ESP32 to deep sleep. Consumes very little power. This is essentialy our power off mode.
    const byte DEFAULT_SCALE_MODE = 0; //0 = nothing special , 1 = auto tare
    const byte DEFAULT_SCALE_UNITS = 0; //0=grams , nothing else is implemented for now
    const byte DEFAULT_ZERO_TRACKING = 0; //in centigrams, 0=disabled
    const byte DEFAULT_SENSITIVITY = 255; //1 = minimum, best for 80SPS, 255 = max, best for 10SPS
    const byte DEFAULT_SMOOTHING = 1; //1 = enabled,0=disabled
    const byte DEFAULT_ADC_SPEED = 10; //10 or 80
    const byte DEFAULT_DECIMAL_DIGITS = 1; //the final rounding before showing to the display
    const byte DEFAULT_BLE_ENABLED = 1;
    const byte DEFAULT_READ_SAMPLES = 1; //how many samples per read
    const byte DEFAULT_STATUS_LED_BUTTON_PRESS = 1; //switch on led on button press
    const byte DEFAULT_STATUS_LED_MAX_VIN = 0; //keep led on when analog VIN >4.3V
    const byte DEFAULT_BATTERY_LEVEL_READ_INTERVAL = 0; //in seconds, not in snooze/standby/deep sleep. By default battery level is only read on startup and if mobile app asks.
    
    const uint32_t DEFAULT_CAL_FACTOR_ULONG = 14000; //actual calfactor you want for your load cell * 10.
    const String DEFAULT_WIFI_HOSTNAME = "OSES";
    const String DEFAULT_WIFI_SSID = "js-iot";
    const String DEFAULT_WIFI_PASSWORD = "iotiscool!22"; //don't worry, I did not forget that :D
    
    
    const byte DEFAULT_STABLE_WEIGHT_DIFF = 5; //in centigrams, 5 is ok for 10SPS and 80 with filtering if using a good load cell
    const byte DEFAULT_ZERO_RANGE = 5; //in centigrams
    const byte DEFAULT_FAKE_STABILITY_RANGE = 0; //in centigrams
};

#endif
