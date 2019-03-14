/*  
  ESPresso Scale PRO
  v1.2 , March 2019

  High(-ish) resolution (0.01g), speed, repeatability scale based on ESP32 MCU and TI ADS1232 ADC.
  Download schematics,gerber,BOM,firmware,3d printed case stl,ionic app from github or fork away !    
  
  Created by John Sartzetakis, Feb 2019
  Released into the public domain.
  https://gitlab.com/jousis/espresso-scale
*/


/*
 * 
 * 
 * This file and a_globals contain many options you should see and change if necessary !
 * Please do not skip them.
 * Finally, the default values for all the options stored in EEPROM is on the settings.h file
 * 
 * 
 */

/*  
///////// WARNING !!! //////////
//       Please read          //
////////////////////////////////
//If you want to use wifi OTA AND BLE, you must select a different partition scheme
//The default one cannot fit the whole sketch.
//For Arduino IDE go to Tools -> Partition Scheme -> Minimal SPIFFS (Large APPS with OTA)
//
//
// Also, optionally, change frequency to 80MHz to reduse power consumption (esp32 is a beast, we don't need full speed).
*/




/* 
  \\\\\\\\\\\\\\\\\\\\    INDEX    ////////////////////
  (use Find on all tabs, including the numbers)

  0.OPTIONS (this file)
  0.1.OPTIONS
  0.2.INCLUDES
  
  1.VARIABLES/CONSTS (a_globals)
  1.1.BLE
  1.2.OTA UPDATE
  1.3.DISPLAY
  1.4.SCALE
  1.5.BUTTONS
  1.6.OTHER

  2.BUTTONS (b_buttons)
  2.1.OPTIONS
  2.2.TOUCH CALLBACKS
  2.3.ISR CALLBACKS
  2.4.ENABLE/DISABLE BUTTONS
  2.5.POWER BUTTON LOGIC
  2.6.TARE BUTTON LOGIC
  2.7.SECONDARY BUTTON LOGIC

  3.SETTINGS API (c_settingsapi)

  4.DISPLAY (d_display)
  4.1.STATUS LED
  4.2.LED SEGMENT OUTPUT
  4.3.POWER ON/OFF

  5.SLEEP (e_sleep)
  5.1.DEEP SLEEP

  6.BLE (f_ble)
  6.1.API
  6.2.CALLBACKS

  7.setup() (g_setup)

  8.loop() (h_loop)
*/



/* 
   \\\\\\\\\\\\\\\\\\\\    0.OPTIONS    ////////////////////
*/

////////// 0.1.OPTIONS //////////
//comment the following if you have the serial pins unconnected.
//Floating serial pins can cause slowdown issues so if you comment SERIAL RX/TX pins are defined as input with INPUT_PULLUP (g_setup).
#define SERIAL
#define DEBUG //enables the serial prints with debug information. Errors are printed with only SERIAL defined.

#ifdef SERIAL
  #define SERIAL_PRINT(x) Serial.print (x)
  #define SERIAL_PRINTLN(x) Serial.println (x)
  #ifdef DEBUG
    #define DEBUG_PRINT(x) Serial.print (x)
    #define DEBUG_PRINTLN(x) Serial.println (x)
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

//Select your PCB version (affects some pin numbers)
#define PRO
//#define LUNAR_ECLIPSE


//Select your screen (comment others)
#define ST7735 //TFT
//#define SSD1306 //old style OLED
//#define SSD1331 //Full color OLED
//#define LEDSEGMENT //Led segment displays with MAX7221 controller

//If you don't care for OTA update, comment the following
#define OTA

#define STATUS_LED //soldered any status led ??




////////// 0.1.INCLUDES //////////
#include <math.h>
#include "SCALE.h"
#include "SETTINGS.h" //very basic wrapper of preferences.h


#ifdef OTA
  //http OTA update
  #include <WiFi.h>
  #include <WiFiClient.h>
  #include <WebServer.h>
  #include <Update.h>
#endif


#ifdef LEDSEGMENT
  //http://wayoda.github.io/LedControl/pages/software
  //https://playground.arduino.cc/Main/LedControl
  #include "LedControl.h" // LED segment
#endif

#ifdef ST7735 // TFT with PWM support
  #include <Adafruit_GFX.h>
  #include <Adafruit_ST7735.h>
#endif

#ifdef SSD1306 // OLED
  #include <Adafruit_GFX.h>
  #include <Adafruit_SSD1306.h>
#endif

#ifdef SSD1331 // Full color OLED
  //Excellent library by .S.U.M.O.T.O.Y., coded by Max MC Costa
  //Download before including
  //https://github.com/sumotoy/SSD_13XX
  #include <SPI.h>
  #include <SSD_13XX.h>
#endif


#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>


SETTINGS settings = SETTINGS();


  
