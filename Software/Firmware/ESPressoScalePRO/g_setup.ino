/* 
   \\\\\\\\\\\\\\\\\\\\    7.setup()    ////////////////////
*/


void setup() {

  #ifdef SERIAL
    Serial.begin(115200);
    SERIAL_PRINTLN("ESPresso Scale");
  #else
    pinMode(1,INPUT_PULLUP);
    pinMode(3,INPUT_PULLUP);
  #endif

  upgradeMode = 0;
  wifiIP = "";
  if (!settings.loadSettings()) {
    DEBUG_PRINTLN("resetting all settings");
    settings.resetSettings();
    //no need to apply settings, we are in setup()
  }

  //TESTING // ALWAYS RESET  
  //resetSettings();

  
  #ifdef SSD1306
    if(!oled.begin(SSD1306_SWITCHCAPVCC)) {
      SERIAL_PRINTLN("SSD1306 allocation failed");
    } else {
      oled.display();
      oled.clearDisplay();
      oled.setRotation(2);
      oled.setTextSize(1);
      oled.setCursor(30, 20);
      oled.setTextColor(WHITE);
      oled.print("Open Source");
      oled.setCursor(35,30);
      oled.print("ESPresso");
      oled.setCursor(40,40);
      oled.print("Scale");
      oled.display();      
    }
  #endif

  #ifdef SSD1331
    oled.begin();
    oled.setRotation(2);
    oled.fillScreen(BLACK);
    oled.setTextScale(1,1);
    oled.setCursor(10, 20);
    oled.setTextColor(RED);
    oled.print("Open Source");
    oled.setCursor(15,30);
    oled.print("ESPresso");
    oled.setCursor(20,40);
    oled.print("Scale");
  #endif

  #ifdef ST7735
    //pinMode(TFT_BACKLIGHT, OUTPUT);
    //digitalWrite(TFT_BACKLIGHT, HIGH); // Backlight on 100%
    ledcAttachPin(TFT_BACKLIGHT_PIN, 1); //Dynamic backlight control , simply call ledcWrite(1,byte);
    ledcSetup(1, 100, 8);
    ledcWrite(1, TFT_FULL_BRIGHTNESS);
    
    tft.initR(INITR_MINI160x80);
    tft.fillScreen(0xFFFFFF);
    tft.setRotation(1);  
    
    tft.setTextSize(2);
    tft.setCursor(18, 10);
    tft.setTextColor(0x000000);
    tft.print("Open Source");
    tft.setCursor(35,30);
    tft.print("ESPresso");
    tft.setCursor(50,50);
    tft.print("Scale");      
  #endif

  #ifdef LEDSEGMENT
    DEBUG_PRINTLN("Initialising LED Display");
    lc.shutdown(LED_ADDR,false);
    /* Set the brightness to a medium values */
    lc.setIntensity(LED_ADDR,LED_BRIGHTNESS_HIGH);
    /* and clear the display */
    lc.clearDisplay(LED_ADDR);
    lc.setChar(LED_ADDR,0,'0',false);
    lc.setChar(LED_ADDR,1,'5',false);
    lc.setChar(LED_ADDR,2,'E',false);
    lc.setChar(LED_ADDR,3,'5',false);
  #endif

  if (settings.bleEnabled) {
    // Create the BLE Device
    BLEDevice::init(BLE_DEVICE_NAME);
  
    // Create the BLE Server
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new bleServerCallbacks());
  
    // Create the BLE Service
    BLEService *pService = pServer->createService(BLE_SERVICE_UUID);
  
    // Create a BLE Characteristic
    pWeightCharacteristic = pService->createCharacteristic(
                        BLE_CHARACTERISTIC_WEIGHT_UUID,
                        BLECharacteristic::PROPERTY_READ   |
                        BLECharacteristic::PROPERTY_NOTIFY
                      );
    pSettingsCharacteristic = pService->createCharacteristic(
                        BLE_CHARACTERISTIC_SETTINGS_UUID,
                        BLECharacteristic::PROPERTY_READ |
                        BLECharacteristic::PROPERTY_WRITE
                      );
                        
    pSettingsCharacteristic->setCallbacks(new bleSettingsCallbacks());
    
    // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
    // Create a BLE Descriptor
    pWeightCharacteristic->addDescriptor(new BLE2902());
    pSettingsCharacteristic->addDescriptor(new BLE2902());
  
    // Start the service
    pService->start();
    
    // Start advertising
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(BLE_SERVICE_UUID);
    pAdvertising->setScanResponse(false);
    pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
    BLEDevice::startAdvertising();
    DEBUG_PRINTLN("BLE // Advertising");
  }


  //initiating OTA
  if (settings.otaUpgrade == 1) {        // Connect to WiFi network
    upgradeMode=1;
    //we got otaUpdate from EEPROM so for this run, we will do what it says.
    //however, we immediately want to reset it on EEPROM so on our next reboot will be 0 again
    settings.saveSettingByte("otaUpgrade",0);
    
    #ifdef OTA
      WiFi.begin(settings.wifiSSID.c_str(), settings.wifiPassword.c_str());
      DEBUG_PRINT("connecting to ");DEBUG_PRINT(settings.wifiSSID);DEBUG_PRINT(" with pass ");DEBUG_PRINTLN(settings.wifiPassword);
  
      if (settings.bleEnabled == 1) {
        pWeightCharacteristic->setValue("UPGRADE");
        pWeightCharacteristic->notify();
      }
  
      uint32_t wifiConnectionStart = millis();
    
      // Wait for connection
      while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        yield();
        DEBUG_PRINT(".");
        if (millis() - wifiConnectionStart > WIFI_CONNECTION_TIMEOUT*1000) {
          ESP.restart();
        }
      }
      SERIAL_PRINTLN("");
      SERIAL_PRINT("Connected to ");
      SERIAL_PRINTLN(settings.wifiSSID);
      SERIAL_PRINT("IP address: ");
      SERIAL_PRINTLN(WiFi.localIP().toString());
      wifiIP = WiFi.localIP().toString();    
  
  
      server.on("/", HTTP_GET, []() {
        server.sendHeader("Connection", "close");
        server.send(200, "text/html", HTTP_SRV_LOGIN_HTML);
      });
      server.on("/main", HTTP_GET, []() {
        server.sendHeader("Connection", "close");
        server.send(200, "text/html", HTTP_SRV_MAIN_HTML);
      });
      /*handling uploading firmware file */
      server.on("/update", HTTP_POST, []() {
        server.sendHeader("Connection", "close");
        server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
        ESP.restart();
      }, []() {
        HTTPUpload& upload = server.upload();
        if (upload.status == UPLOAD_FILE_START) {
          Serial.printf("Update: %s\n", upload.filename.c_str()); 
          #ifdef LEDSEGMENT
            lc.clearDisplay(LED_ADDR); 
            //G
            lc.setRow(LED_ADDR,1,B01011111);
            lc.setChar(LED_ADDR,2,'0',false);
            lc.setChar(LED_ADDR,3,'0',false);
            lc.setChar(LED_ADDR,4,'d',false);
          #endif
          #ifdef ST7735
            tft.fillScreen(0xFFFFFF);
            tft.setTextSize(2);
            tft.setCursor(20, 10);
            tft.setTextColor(0x000000);
            tft.print("UPGRADING");
            tft.setTextSize(1);
            tft.setCursor(0,50);
            tft.print(wifiIP);
          #endif
          #ifdef SSD1306
            oled.clearDisplay();
            oled.setTextSize(2);
            oled.setCursor(18, 10);
            oled.setTextColor(WHITE);
            oled.print("UPGRADE F/W");
            oled.setTextSize(1);
            oled.setCursor(0,30);
            oled.print(wifiIP);
            oled.display();
          #endif
          if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
            Update.printError(Serial);
          }
        } else if (upload.status == UPLOAD_FILE_WRITE) {
          /* flashing firmware to ESP*/
          if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
            Update.printError(Serial);
          }
        } else if (upload.status == UPLOAD_FILE_END) {
          if (Update.end(true)) { //true to set the size to the current progress
            Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
          } else {
            Update.printError(Serial);
          }
        }
      });
      server.begin();  
     #endif
  } else {
    //only proceed if not on OTA upgrade mode
    //Enable the Analog voltage LDO
    #ifdef PRO
      pinMode(ADC_LDO_EN_PIN, OUTPUT);
      digitalWrite(ADC_LDO_EN_PIN,ADC_LDO_ENABLE);
    #endif
  
    delay(250);
  
    scale.begin();
    
    //Apply scale options we got from EEPROM
    scale.zeroRange = (float)settings.zeroRange/100.0;
    scale.fakeStabilityRange = (float)settings.fakeRange/100.0;
    scale.decimalDigits = settings.decimalDigits;
    scale.setSpeed(settings.adcSpeed);
    scale.setSensitivity(settings.sensitivity);
    scale.setSmoothing(settings.smoothing);
    scale.setCalFactor(settings.calFactorULong/10.0);
  
  }

  //Status LED
  #ifdef STATUS_LED
    pinMode(STATUS_LED_PIN,OUTPUT);    
  #endif

  //buttons
  if (POWER_BUTTON_PIN > 0) {
    #ifdef TOUCH
      touchAttachInterrupt(POWER_BUTTON_PIN, touchPowerCallback, TOUCH_POWER_THRESHOLD);
    #endif
    #ifdef BUTTON
      pinMode(POWER_BUTTON_PIN,INPUT_RESISTOR);
      attachInterrupt(digitalPinToInterrupt(POWER_BUTTON_PIN), powerButtonISR, INTERRUPT_MODE);
    #endif
  }
  if (TARE_BUTTON_PIN > 0) {
    #ifdef TOUCH
      touchAttachInterrupt(TARE_BUTTON_PIN, touchTareCallback, TOUCH_TARE_THRESHOLD);
    #endif
    #ifdef BUTTON
      pinMode(TARE_BUTTON_PIN,INPUT_RESISTOR);
      attachInterrupt(digitalPinToInterrupt(TARE_BUTTON_PIN), tareButtonISR, INTERRUPT_MODE);
    #endif
  }
  if (SECONDARY1_BUTTON_PIN > 0) {
    #ifdef TOUCH
      touchAttachInterrupt(SECONDARY1_BUTTON_PIN, touchSecondary1Callback, TOUCH_SECONDARY1_THRESHOLD);
    #endif
    #ifdef BUTTON
      pinMode(SECONDARY1_BUTTON_PIN,INPUT_RESISTOR);
      attachInterrupt(digitalPinToInterrupt(SECONDARY1_BUTTON_PIN), secondary1ButtonISR, INTERRUPT_MODE);
    #endif
  }
    

  initialising = true;  
  #ifdef TOUCH
    esp_sleep_enable_touchpad_wakeup();
  #endif
  #ifdef BUTTON
    esp_sleep_enable_ext0_wakeup(POWER_BUTTON_PIN_RTC, 1); //1 = High, 0 = Low
  #endif
  lastActionMillis = millis();
  

  
}
