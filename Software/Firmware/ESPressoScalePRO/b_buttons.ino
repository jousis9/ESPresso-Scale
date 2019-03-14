/* 
   \\\\\\\\\\\\\\\\\\\\    2.BUTTONS    ////////////////////
*/

////////// 2.1.OPTIONS //////////
const uint16_t BUTTON_HOLD_TIME_IGNORE = 10; //anything below 10ms is ignored
const uint16_t BUTTON_HOLD_TIME_LONG = 3000; // in ms, below that is simple click, above or equal is long click
const uint16_t BUTTON_DEBOUNCE_INTERVAL = 200; //in ms
uint32_t lastButtonDebounceUpdate = 0;

//true if user still touching, false if not => true is set in interrupt callback
bool powerButtonPressing = false;
bool tareButtonPressing = false;
bool secondary1ButtonPressing = false;

//millis from 1st touch detected => set in interrupt callback
uint32_t powerButtonPressedStartTime = 0;
uint32_t tareButtonPressedStartTime = 0;
uint32_t secondary1ButtonPressedStartTime = 0;




////////// 2.2.TOUCH CALLBACKS //////////
void touchPowerCallback(){
  powerButtonPressing = true;  
  if (powerButtonPressedStartTime <= 0) {
    powerButtonPressedStartTime = millis();
  }
}

void touchTareCallback(){
  tareButtonPressing = true;
  if (tareButtonPressedStartTime <= 0) {
    tareButtonPressedStartTime = millis();
  }
}

void touchSecondary1Callback(){
  secondary1ButtonPressing = true;
  if (secondary1ButtonPressedStartTime <= 0) {
    secondary1ButtonPressedStartTime = millis();
  }
}



////////// 2.3.ISR CALLBACKS //////////
void IRAM_ATTR powerButtonISR() {
  powerButtonPressing = true;
  if (powerButtonPressedStartTime <= 0) {
    powerButtonPressedStartTime = millis();
  }
}

void IRAM_ATTR tareButtonISR() {
  tareButtonPressing = true;  
  if (tareButtonPressedStartTime <= 0) {
    tareButtonPressedStartTime = millis();
  }
}
void IRAM_ATTR secondary1ButtonISR() {
  secondary1ButtonPressing = true;  
  if (secondary1ButtonPressedStartTime <= 0) {
    secondary1ButtonPressedStartTime = millis();
  }
}




////////// 2.4.ENABLE/DISABLE BUTTONS //////////
void disableSecondaryButtons() {
  //disable all touch except power
  //for regular buttons ?? anything we should do here??
  #ifdef TOUCH
    if (TARE_BUTTON_PIN > 0) {
      touchAttachInterrupt(TARE_BUTTON_PIN, touchTareCallback, 0);
    }
    if (SECONDARY1_BUTTON_PIN > 0) {
      touchAttachInterrupt(SECONDARY1_BUTTON_PIN, touchSecondary1Callback, 0);
    }
  #endif
}
void enableSecondaryButtons() {
  //disable all touch except power
  #ifdef TOUCH
    if (TARE_BUTTON_PIN > 0) {
      touchAttachInterrupt(TARE_BUTTON_PIN, touchTareCallback, TOUCH_TARE_THRESHOLD);
    }
    if (SECONDARY1_BUTTON_PIN > 0) {
      touchAttachInterrupt(SECONDARY1_BUTTON_PIN, touchSecondary1Callback, TOUCH_SECONDARY1_THRESHOLD);
    }
  #endif
}


////////// 2.5.POWER BUTTON LOGIC //////////
void handlePowerButton() {

  #ifdef BUTTON
    powerButtonPressing = digitalRead(POWER_BUTTON_PIN);
  #endif

  if (settings.slBtnPress == 1) {
    handleStatusLed(powerButtonPressing);
  }
  if (powerButtonPressedStartTime>0) {
    uint32_t holdtime = millis() - powerButtonPressedStartTime;
    if (!powerButtonPressing) {
      //previously true, now false
      if (holdtime > BUTTON_HOLD_TIME_IGNORE) {
        DEBUG_PRINTLN("Touch of power button");      
        if (lightSleep || snooze) {
          wakeup = true;
          //beginTare = true;
        }
      }
      powerButtonPressedStartTime = 0; //reset
    } else {
      //detect long/special touch even before releasing
      if (holdtime > BUTTON_HOLD_TIME_LONG) {
        //check if tare is also touched
        holdtime = millis() - tareButtonPressedStartTime;
        if (tareButtonPressing && holdtime > BUTTON_HOLD_TIME_IGNORE) {
          //initiate firmware upgrade
          powerButtonPressedStartTime = 0; //reset
          DEBUG_PRINTLN("Initiating Upgrade...");
          settings.saveSettingByte("otaUpgrade",1);
          delay(1000);
          ESP.restart();
        } else {
          powerButtonPressedStartTime = 0; //reset
          DEBUG_PRINTLN("Long touch of power button");
          initiateDeepSleep();
          delay(1000);          
        }
        
      }
    }   
    lastActionMillis = millis();
  }
  #ifdef TOUCH
    powerButtonPressing = false; //always reset
  #endif
}

////////// 2.6.TARE BUTTON LOGIC //////////
void handleTareButton() {   
  
  #ifdef BUTTON
    tareButtonPressing = digitalRead(TARE_BUTTON_PIN);
  #endif
  if (settings.slBtnPress == 1) {
    handleStatusLed(powerButtonPressing);
  }
  
  if (tareButtonPressedStartTime>0) {
    uint32_t holdtime = millis() - tareButtonPressedStartTime;
    if (!tareButtonPressing) {
      //previously true, now false
      if (holdtime > BUTTON_HOLD_TIME_IGNORE) {
        DEBUG_PRINTLN("Touch of tare button");    
        scale.tare();  
        if (lightSleep || snooze) {
          wakeup = true;
          //beginTare = true;
        }
      }
      tareButtonPressedStartTime = 0; //reset
    } else {
      //detect long/special touch even before releasing
      if (holdtime > BUTTON_HOLD_TIME_LONG) {
        tareButtonPressedStartTime = 0; //reset
        DEBUG_PRINTLN("Long touch of tare button");
        delay(1000);
      }
    }   
    lastActionMillis = millis();
  }
  #ifdef TOUCH
    tareButtonPressing = false; //always reset
  #endif
}


////////// 2.7.SECONDARY BUTTON LOGIC //////////
void handleSecondary1Button() {
  #ifdef BUTTON
    secondary1ButtonPressing = digitalRead(SECONDARY1_BUTTON_PIN);
  #endif
  if (settings.slBtnPress == 1) {
    handleStatusLed(secondary1ButtonPressing);
  }
  
  if (secondary1ButtonPressedStartTime>0) {
    uint32_t holdtime = millis() - secondary1ButtonPressedStartTime;
    if (!secondary1ButtonPressing) {
      //previously true, now false
      if (holdtime > BUTTON_HOLD_TIME_IGNORE) {
        DEBUG_PRINTLN("Touch of secondary1 button");    
        if (lightSleep || snooze) {
          wakeup = true;
        }
      }
      secondary1ButtonPressedStartTime = 0; //reset
    } else {
      if (holdtime > BUTTON_HOLD_TIME_LONG) {
        secondary1ButtonPressedStartTime = 0; //reset
        DEBUG_PRINTLN("Long touch of secondary1 button");
        delay(1000);
      }
    }   
    lastActionMillis = millis();
  }
  #ifdef TOUCH
    secondary1ButtonPressing = false; //always reset
  #endif
  
}
