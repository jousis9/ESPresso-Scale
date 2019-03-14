/* 
   \\\\\\\\\\\\\\\\\\\\    3.SETTINGS API    ////////////////////
*/


//below are all the commands the user can read/write through BLE/Serial
String handleCommand(String cmd, String val) {
  bool getCommand = val.indexOf("get") >= 0; 
  if (cmd == "tare") {
    scale.tare();
  } else if (cmd == "calibrate") {
    calibrateToUnits =  strtof(val.c_str(),NULL);
  } else if (cmd == "reboot") {
    ESP.restart();
  } else if (cmd == "deepSleep") {
    initiateDeepSleep();
  } else if (cmd == "resetSettings") {
    settings.resetSettings();
  } else if (cmd == "upgradeMode") {
    if (getCommand) {
      return String(upgradeMode);
    }    
  } else if (cmd == "wifiIP") {
    if (getCommand) {
      return wifiIP;
    }
  } else if (cmd == "otaUpgrade") {
    if (getCommand) {
      return String(settings.otaUpgrade);
    } else {
      uint8_t value = atoi(val.c_str());
      if (value == 1) {
        //specifically check if 1, else =0 ignore all other values;
        settings.saveSettingByte("otaUpgrade",1);
        delay(1000);
        ESP.restart();
      } else {
        settings.saveSettingByte("otaUpgrade",0);
      }
    }
  } else if (cmd == "snoozeTimeout") {
    if (getCommand) {
      return String(settings.snoozeTimeout);
    } else {
      uint8_t value = atoi(val.c_str());
      settings.snoozeTimeout = value;
      settings.saveSettingByte("snoozeTimeout",value);
    }
  } else if (cmd == "lightSleepTimeout") {
    if (getCommand) {
      return String(settings.lightSleepTimeout);
    } else {
      uint8_t value = atoi(val.c_str());
      settings.lightSleepTimeout = value;
      settings.saveSettingByte("lightSleepTimeout",value);
    }
  } else if (cmd == "deepSleepTimeout") {
    if (getCommand) {
      return String(settings.deepSleepTimeout);
    } else {
      uint8_t value = atoi(val.c_str());
      settings.deepSleepTimeout = value;
      settings.saveSettingByte("deepSleepTimeout",value);
    }
  } else if (cmd == "scaleMode") {
    if (getCommand) {
//      return String(scale.scaleMode);
      return String(settings.scaleMode);
    } else {
      uint8_t value = atoi(val.c_str());
      settings.scaleMode = value;
      scale.setScaleMode(value);
      settings.saveSettingByte("scaleMode",value);
    }    
  } else if (cmd == "scaleUnits") {
    if (getCommand) {
      return String(settings.scaleUnits);
    } else {
      uint8_t value = atoi(val.c_str());
      settings.scaleUnits = value;
      settings.saveSettingByte("scaleUnits",value);      
    }
  } else if (cmd == "zeroTracking") {
    if (getCommand) {
      return String((int)(scale.zeroTracking*100.0));
    } else {
      uint8_t value = atoi(val.c_str());
      settings.zeroTracking = value;
      scale.zeroTracking = (float)value/100.0;
      settings.saveSettingByte("zeroTracking",value);
    }    
  } else if (cmd == "zeroRange") {
    if (getCommand) {
      return String((int)(scale.zeroRange*100.0));
    } else {
      uint8_t value = atoi(val.c_str());
      settings.zeroRange = value;
      scale.zeroRange = (float)value/100.0;
      settings.saveSettingByte("zeroRange",value);
    }
  } else if (cmd == "fakeRange") {
    if (getCommand) {
      return String((int)(scale.fakeStabilityRange*100.0));
    } else {
      uint8_t value = atoi(val.c_str());
      settings.fakeRange = value;
      scale.fakeStabilityRange = (float)value/100.0;
      settings.saveSettingByte("fakeRange",value);
    }
  } else if (cmd == "stableWeightDiff") {
    if (getCommand) {
      return String((int)(scale.stableWeightDiff*100.0));
    } else {
      uint8_t value = atoi(val.c_str());
      settings.stableWeightDiff = value;
      scale.stableWeightDiff = (float)value/100.0;
      settings.saveSettingByte("stableWeightDiff",value);
    }    
  } else if (cmd == "sensitivity") {
    if (getCommand) {
      return String(settings.sensitivity);
    } else {
      uint8_t value = atoi(val.c_str());
      settings.sensitivity = value;
      scale.setSensitivity(value);
      settings.saveSettingByte("sensitivity",value);
    }
  } else if (cmd == "smoothing") {
    if (getCommand) {      
      return String(scale.getSmoothing());
    } else {
      uint8_t value = atoi(val.c_str());
      settings.smoothing = value;
      scale.setSmoothing(value);
      settings.saveSettingByte("smoothing",value);
    }
  } else if (cmd == "adcSpeed") {
    if (getCommand) {
      return String(scale.getSpeed());
    } else {
      uint8_t value = atoi(val.c_str());
      settings.adcSpeed = value;
      scale.setSpeed(value);
      settings.saveSettingByte("adcSpeed",value);
    }
  } else if (cmd == "decimalDigits") {
    if (getCommand) {
      return String(scale.decimalDigits);
    } else {
      uint8_t value = atoi(val.c_str());
      settings.decimalDigits = value;
      scale.decimalDigits = value;
      settings.saveSettingByte("decimalDigits",value);
    }
  } else if (cmd == "bleEnabled") {
    if (getCommand) {
      return String(settings.bleEnabled);
    } else {
      uint8_t value = atoi(val.c_str());
      settings.saveSettingByte("bleEnabled",value);
      ESP.restart();
    }
  } else if (cmd == "readSamples") {
    if (getCommand) {
      return String(settings.readSamples);
    } else {
      uint8_t value = atoi(val.c_str());
      settings.readSamples = value;
      settings.saveSettingByte("readSamples",value);
    }
  } else if (cmd == "slBtnPress") {
    if (getCommand) {
      return String(settings.slBtnPress);
    } else {
      uint8_t value = atoi(val.c_str());
      settings.slBtnPress = value;
      settings.saveSettingByte("slBtnPress",value);
    }
  } else if (cmd == "slMaxVIN") {
    if (getCommand) {
      return String(settings.slMaxVIN);
    } else {
      uint8_t value = atoi(val.c_str());
      settings.slMaxVIN = value;
      settings.saveSettingByte("slMaxVIN",value);
    }
  } else if (cmd == "batReadInterval") {
    if (getCommand) {
      return String(settings.batReadInterval);
    } else {
      uint8_t value = atoi(val.c_str());
      settings.batReadInterval = value;
      settings.saveSettingByte("batReadInterval",value);
    }    
  } else if (cmd == "calFactor") {
    if (getCommand) {
      return String(scale.getCalFactor());
    } else {
      float value = strtof(val.c_str(),NULL);
      uint32_t valLng = value*10;
      settings.saveSettingByte("calFactorULong",valLng); 
      scale.setCalFactor(value);
    }
  } else if (cmd == "wifiSSID") {
    if (getCommand) {
      return settings.wifiSSID;
    } else {
      settings.wifiSSID = val.c_str();
      DEBUG_PRINT("our new value for ssid is ");DEBUG_PRINTLN(val);
      settings.saveSettingString("wifiSSID",val.c_str()); 
    }
  } else if (cmd == "wifiPassword") {
    if (getCommand) {
      return settings.wifiPassword;
    } else {
      settings.wifiPassword = val.c_str();
      settings.saveSettingString("wifiPassword",val.c_str()); 
    }
  } else {
    //not a known command
    return "UNKNOWN";
  }
  return "OK";
}
