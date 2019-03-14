/* 
   \\\\\\\\\\\\\\\\\\\\    4.DISPLAY    ////////////////////
*/

uint32_t lastDisplayUpdate = 0;
#ifdef STATUS_LED
  bool lastStatusLedValue = 0;  
#endif

////////// 4.1.STATUS LED //////////
void handleStatusLed(bool value) {
  #ifdef STATUS_LED
     if ( lastStatusLedValue != value ) {
      digitalWrite(STATUS_LED_PIN,value);
      lastStatusLedValue = value;
     }
  #endif
}


////////// 4.2.LED SEGMENT OUTPUT //////////
//Function responsible of printing the final weight in our LED segments
//It will automatically remove decimals if we run out of space
#ifdef LEDSEGMENT
  void writeTo7Seg(double number, byte noOfDigits, byte noOfDecimal) {  
      lc.clearDisplay(LED_ADDR);      
      char buffer[noOfDigits * 2]; //just to be safe
      dtostrf (number, noOfDigits, noOfDecimal, buffer);
      byte startat = 0;   
      byte digitNo = 0;
      byte dp = false;  
      //dtostrf considers the dot a separate byte
      //but for us (led segment) is just a flag on the previous byte
      //one way to solve this issue is to offset the counter. So, with only 1 for loop and an extra if, we are good.
      //If you prefer, you can loop the byte array once and construct a second byte array and a true/false flag for the dp
      int startcount=0;
      if (noOfDecimal == 0) {
        startcount=1;
      }      
      for (int i = startcount; i < noOfDigits * 2; i++) {
        if (i < noOfDigits-1 && buffer[i+1] == '.') {
          dp = true;
        }
        if (buffer[i] != '.') {
          lc.setChar(LED_ADDR,digitNo,buffer[i],dp);
          if (dp) {
            dp = false;
          }
          digitNo++;
        }
        if (digitNo == noOfDigits) {
          break;
        }
      }
  }
#endif


////////// 4.3.POWER ON/OFF //////////
void wakeUpDisplay() {
  #ifdef ST7735
    ledcAttachPin(TFT_BACKLIGHT_PIN, 1); //Dynamic backlight control , simply call ledcWrite(1,byte);
    ledcSetup(1, 100, 8);
    ledcWrite(1, TFT_FULL_BRIGHTNESS);    
  #endif
}

void shutDownDisplay() {
  #ifdef LEDSEGMENT
    lc.clearDisplay(LED_ADDR);  
    lc.setChar(LED_ADDR,0,'-',false);
    lc.setChar(LED_ADDR,2,'-',false);
    lc.setChar(LED_ADDR,4,'-',false);
    //briefly pause so the user can see we got his command
    delay(1000);
    lc.clearDisplay(LED_ADDR);
  #endif
  #ifdef ST7735
    tft.fillScreen(0xFFFFFF);
    tft.setTextSize(2);
    tft.setCursor(20, 10);
    tft.setTextColor(0x000000);
    tft.print("ZZZzzzz");    
    delay(1000);
    tft.fillScreen(0xFFFFFF);
    ledcWrite(1, 0);
    //detach pwm
    ledcDetachPin(TFT_BACKLIGHT_PIN); 
    delay(100); 
    pinMode(TFT_BACKLIGHT_PIN, OUTPUT);
    delay(100);
    digitalWrite(TFT_BACKLIGHT_PIN,0); //shut down backlight completely
    delay(100);
  #endif  
}
