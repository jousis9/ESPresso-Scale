/* 
   \\\\\\\\\\\\\\\\\\\\    5.SLEEP    ////////////////////
*/

////////// 5.1.DEEP SLEEP //////////
void initiateDeepSleep() {
  //We must inform the user, wait to debounce the button and then sleep.
  //if we sleep while power is pressed, we will wake up again.
  //shutdown display immediately
  shutDownDisplay();
  #ifdef STATUS_LED
    //pulse status led until the user releases the power button    
    bool value = false;
    while(digitalRead(POWER_BUTTON_PIN)) {
      value = !value;
      handleStatusLed(value);
//      delay(500);
    }
  #endif
  #ifdef PRO
    //disable our LDO
    digitalWrite(ADC_LDO_EN_PIN,ADC_LDO_DISABLE);
  #endif
  disableSecondaryButtons();
  DEBUG_PRINTLN("bye");
  #ifdef SERIAL
    Serial.flush();
  #endif
  delay(250);
  esp_deep_sleep_start();
}


////////// 5.2.LIGHT SLEEP //////////
void initiateLightSleep() {
  DEBUG_PRINTLN("shutting down ADC,dimming display");
  #ifdef PRO
    //disable our LDO
    digitalWrite(ADC_LDO_EN_PIN,ADC_LDO_DISABLE);
  #endif
  disableSecondaryButtons();
  lightSleep = true;
  wakeup = false;
  scale.powerOff();
  #ifdef ST7735
    shutDownDisplay();
  #endif         
  #ifdef LEDSEGMENT
    lc.setIntensity(LED_ADDR,LED_BRIGHTNESS_LOW);        
  #endif
  //https://github.com/espressif/esp-idf/issues/2070
  //btStop();
}

void wakeUpFromLightSleep() {
  DEBUG_PRINTLN("Should wake up now");
  #ifdef PRO
    //disable our LDO
    digitalWrite(ADC_LDO_EN_PIN,ADC_LDO_ENABLE);
  #endif
  delay(50);
  enableSecondaryButtons();
  lightSleep = false;
  snooze = false;
  wakeup = false;
  scale.powerOn();
  #ifdef ST7735
    wakeUpDisplay();
  #endif
  #ifdef LEDSEGMENT
    lc.setIntensity(LED_ADDR,LED_BRIGHTNESS_HIGH);
  #endif
  //https://github.com/espressif/esp-idf/issues/2070
  //btStart();
  lastActionMillis = millis();  
}
