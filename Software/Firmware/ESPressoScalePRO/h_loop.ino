/* 
   \\\\\\\\\\\\\\\\\\\\    8.loop()    ////////////////////
*/
void loop() {
  #ifdef SERIAL
    if (Serial.available() > 0) {
      int INPUT_SIZE = 30;
      char input[INPUT_SIZE + 1];
      byte size = Serial.readBytes(input, INPUT_SIZE);
      input[size] = 0;
      char* separator = strchr(input, '=');    
      if (separator != 0)
      {
          *separator = 0;
          String command = input;
          ++separator;        
          if (strlen(separator) > 1) {
            separator[strlen(separator)-1] = '\0';
            DEBUG_PRINT("command ");DEBUG_PRINTLN(command);
            DEBUG_PRINTLN(handleCommand(command,separator));
          }
      }
    }
  #endif
  if (settings.otaUpgrade == 1) {    
    if (initialising) {
      initialising = false;
      #ifdef ST7735
        tft.fillScreen(0xFFFFFF);        
        tft.setTextSize(2);
        tft.setCursor(25, 10);
        tft.setTextColor(0x000000);
        tft.print("UPGRADE");
        tft.setCursor(40,30);
        tft.print("MODE");
        tft.setCursor(0,50);
        tft.print(wifiIP);   
      #endif
      #ifdef LEDSEGMENT
        lc.clearDisplay(LED_ADDR); 
        //uppercase u letter in digit 1
        lc.setRow(LED_ADDR,1,B00111110);
        lc.setChar(LED_ADDR,2,'P',false);
        lc.setChar(LED_ADDR,3,'d',false);
      #endif
    }
    server.handleClient();
    handlePowerButton();
    delay(100);
    return;
  }

  




  if (millis() > (lastButtonDebounceUpdate + BUTTON_DEBOUNCE_INTERVAL)) {
    lastButtonDebounceUpdate += BUTTON_DEBOUNCE_INTERVAL;
    if (POWER_BUTTON_PIN>0){ handlePowerButton(); }
    if (TARE_BUTTON_PIN>0){ handleTareButton(); }
    if (SECONDARY1_BUTTON_PIN>0){ handleSecondary1Button(); }
  }



  if (lightSleep && wakeup) {    
    wakeUpFromLightSleep();
  }

  if (snooze && wakeup) {
    snooze = false;
    wakeup = false;
    #ifdef ST7735
      ledcWrite(1, TFT_FULL_BRIGHTNESS);
    #endif
    #ifdef LEDSEGMENT
      lc.setIntensity(LED_ADDR,LED_BRIGHTNESS_HIGH);
    #endif
    lastActionMillis = millis();
  }
  

  // !lightSleep if
  // Don't do any of the following if already sleeping in order to conserve power
  // If you need something to update during light sleep, remove it from the following if
  // The only way to wake up from light sleep is if you set wakeup = true , ex. from a button or from touch interrupt (or any other interrupt)
  // PRO pcb consumes about 40mA during light sleep
  if (!lightSleep) {
    //While initialising, keep taring... (you can comment that, ADS1232 is fine with only the basic initialising done on the library)
    if (millis()>INITIALISING_SECS*1000 && initialising) {
      initialising = false;
      #ifdef ST7735
        tft.fillScreen(0xFFFFFF); //black
      #endif
    }
    if (initialising) {
      beginTare = true;     
      #ifdef LEDSEGMENT
        lc.clearDisplay(LED_ADDR); 
        lc.setChar(LED_ADDR,0,'0',false);
        lc.setChar(LED_ADDR,1,'5',false);
        lc.setChar(LED_ADDR,2,'E',false);
        lc.setChar(LED_ADDR,3,'5',false);
      #endif
    }
  
  
    if (beginTare) {
      beginTare = false;
      scale.calibrateADC();
      scale.tare();
      DEBUG_PRINTLN("taring...");
    }
    
    double grams = 0;
    grams = scale.readUnits(settings.readSamples);   

    DEBUG_PRINT(grams);DEBUG_PRINT("g");
    if (scale.hasSettled) {
      DEBUG_PRINT(" (s) - (");    
    } else {
      DEBUG_PRINT(" - (");
    }
    DEBUG_PRINT(scale.lastTareWeightRounded);DEBUG_PRINT("g)   ///   ");
    DEBUG_PRINT(scale.roc);DEBUG_PRINT("g/s // ");
    DEBUG_PRINT(scale.getAdcActualSPS());DEBUG_PRINTLN("SPS");

    if (calibrateToUnits > 0.0) {
      lastActionMillis = millis();
      DEBUG_PRINT("should calibrate to ");DEBUG_PRINTLN(calibrateToUnits);
      #ifdef ST7735
        tft.fillScreen(0xFFFFFF); //black
        tft.setTextColor(0x000000);
        tft.setTextWrap(true);
        tft.setTextSize(1);
        tft.setCursor(1, 10);
        tft.print("Place ");
        tft.setCursor(5, 30);
        tft.print(calibrateToUnits);tft.print("g");
        tft.setCursor(1, 50);
        tft.setTextSize(1);
        tft.print("in the scale and wait");
      #endif
      #ifdef LEDSEGMENT
        writeTo7Seg(calibrateToUnits,6,0);
        lc.setChar(LED_ADDR,0,'C',false);
      #endif
      
      scale.calibrate(calibrateToUnits,30000,0.05);
      DEBUG_PRINT("our old calfactor ");DEBUG_PRINT(settings.calFactorULong/10.0);
      settings.calFactorULong = (uint32_t)(scale.getCalFactor()*10.0);
      DEBUG_PRINT(" our new calfactor ");DEBUG_PRINTLN(settings.calFactorULong/10.0);
      settings.saveSettingULong("calFactorULong",settings.calFactorULong);
      calibrateToUnits = 0.0;
      #ifdef SSD1306
          oled.clearDisplay(); 
          oled.setTextSize(2);
          oled.setTextColor(WHITE);
          oled.setCursor(30,40);
          oled.print("THANKS !");
          oled.display();
      #endif
      #ifdef ST7735
        tft.fillScreen(0xFFFFFF); //black
        tft.setTextColor(0x000000);
        tft.setTextWrap(true);
        tft.setTextSize(2);
        tft.setCursor(1, 30);
        tft.print("THANKS !");
        delay(1000);
        tft.fillScreen(0xFFFFFF); //black
      #endif
    }
  
    if (initialising) {
      lastActionMillis = millis();
      scale.tare();
    }
  
  
    if (millis() > (lastDisplayUpdate + DISPLAY_REFRESH_INTERVAL)) {
      lastDisplayUpdate += DISPLAY_REFRESH_INTERVAL;      
      if (!initialising) {
        #ifdef SSD1306
          oled.clearDisplay(); 
          oled.setTextSize(2);
//          oled.setTextColor(WHITE);
          oled.setCursor(0,20);          
          oled.print(String(grams)+ "g");
          oled.display();
       #endif
        #ifdef SSD1331
          oled.fillScreen(BLACK);
          oled.setTextScale(2,2);
          oled.setTextColor(YELLOW);
          oled.setCursor(0,20);          
          oled.print(String(grams)+ "g");
       #endif
       #ifdef ST7735       
          //tft.fillScreen(0xFFFFFF); //black
          tft.fillRect(0,20,160,25,0xFFFFFF);
          tft.setTextWrap(false);
          tft.setTextSize(3);
          tft.setCursor(10, 20);
  //        tft.print(String(grams)+ "g");    
          char buffer[10];
          for (byte i=0;i<sizeof(buffer);i++) {
            buffer[i] = 'c';
          }
          dtostrf (grams, 6, scale.decimalDigits, buffer);        
          for (byte i=0; i<sizeof(buffer);i++) {
            if ( buffer[i] != 'c'){
              tft.print(String(buffer[i]));
            } else if (buffer[i] == 'c') {
              tft.print("g");
              break;
            }
          }
       #endif    
       #ifdef LEDSEGMENT
          writeTo7Seg(grams,6,scale.decimalDigits);
       #endif
          
      } 
  
    }
    


  if (settings.bleEnabled) {
    //BLE notify loop 
    if (deviceConnected && (millis() > lastBleUpdate + BLE_REFRESH_INTERVAL)) {
      lastBleUpdate += BLE_REFRESH_INTERVAL;
          //pWeightCharacteristic->setValue(grams);
          String gramsstr = String(grams);
          pWeightCharacteristic->setValue(gramsstr.c_str());
          pWeightCharacteristic->notify();
    }
      // BLE disconnecting
      if (!deviceConnected && oldDeviceConnected) {
          delay(500); // give the bluetooth stack the chance to get things ready
          pServer->startAdvertising(); // restart advertising
          DEBUG_PRINTLN("start advertising");
          oldDeviceConnected = deviceConnected;
      }
      // BLE connecting
      if (deviceConnected && !oldDeviceConnected) {
          // do stuff here on connecting
          oldDeviceConnected = deviceConnected;
      }
  }
    

    if (scale.hasSettled) {
      //enable the following check if you want to only snooze when there is no weight on the scale
//      if (fabs(grams) < 10){
////        DEBUG_PRINTLN("don't leave me alone please");
//      } else {
//        wakeup = true;
//        lastActionMillis = millis();      
//      }
    } else {
      wakeup = true;
      lastActionMillis = millis();
    }



    if (settings.lightSleepTimeout > 0 && (millis() - lastActionMillis) >  settings.lightSleepTimeout*10000) {
      initiateLightSleep();
    }

    
    if (settings.snoozeTimeout > 0 && !snooze && ((millis() - lastActionMillis) >  settings.snoozeTimeout*10000)) {
      #ifdef ST7735
        ledcWrite(1, TFT_DIM_BRIGHTNESS); // dimming           
      #endif         
      #ifdef LEDSEGMENT
        lc.setIntensity(LED_ADDR,LED_BRIGHTNESS_LOW);        
      #endif
      DEBUG_PRINTLN("ZZZZzzzzz");
      snooze = true;
      wakeup = false;
    }

    
  //END OF !lightSleep if   
  }


  if (settings.deepSleepTimeout > 0 && (millis() - lastActionMillis) >  settings.deepSleepTimeout*10000) {
    initiateDeepSleep();
  }

  if (snooze) {
    delay(250);
  }
  if (lightSleep) {
    delay(1000);
  }

}
