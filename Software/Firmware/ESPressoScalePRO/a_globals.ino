/* 
   \\\\\\\\\\\\\\\\\\\\    1.VARIABLES/CONSTS    ////////////////////
*/
////////// 1.1.BLE //////////

const int BLE_REFRESH_INTERVAL = 250; //in millis


//random uuids, please do not use them in your final (commercial) product
//https://www.bluetooth.com/specifications/gatt/services
//0x181D => Weight Scale
#define BLE_DEVICE_NAME "OSES"
#define BLE_SERVICE_UUID "b75d181d-e85e-11e8-9f32-f2801f1b9fd1"
#define BLE_CHARACTERISTIC_WEIGHT_UUID "b75d2a9d-e85e-11e8-9f32-f2801f1b9fd1"
#define BLE_CHARACTERISTIC_SETTINGS_UUID "b75d2a9e-e85e-11e8-9f32-f2801f1b9fd3"


////////// 1.2.OTA UPDATE //////////
//           WARNING !!!          //
////////////////////////////////////
//If you want to use wifi OTA AND BLE, you must select a different partition scheme
//The sketch will not fit on the default one.
//For Arduino IDE go to Tools -> Partition Scheme -> Minimal SPIFFS (Large APPS with OTA)
#ifdef OTA
  WebServer server(80);
  byte upgradeMode = 0; //1 if in upgrade mode
  String wifiIP = "";
  static byte WIFI_CONNECTION_TIMEOUT = 30; //in seconds, if we do not connect within 30s, reboot
  
  const char* HTTP_SRV_LOGIN_HTML =
    #include "loginhtml.h"
  ;
  
  const char* HTTP_SRV_MAIN_HTML =
    #include "indexhtml.h"
  ;
#endif


////////// 1.3.DISPLAY //////////
//10fps (100ms) refresh if fine for 10SPS ADC speed if you need to show them all.
//8fps (125ms) is the threshold (IMO) for "look how fast it is" user reaction.
const int DISPLAY_REFRESH_INTERVAL = 100; //in millis => 10fps

#ifdef STATUS_LED
  const byte STATUS_LED_PIN = 2;
#endif



// HARDWARE SPI /// PLEASE READ ///
//
//In PRO pcb the display is indeed connected to ESP32 hardware SPI but not the "default" one for Arduino IDE
//If you wish to use (and you should) hardware spi, please edit the pinout header file accordingly
//Location in windows10 :
//C:\Users\YOURUSERNAME\AppData\Local\Arduino15\packages\esp32\hardware\esp32\LIBRARY-VERSION\variants\BOARDNAME\pins_arduino.h
//so, for ESP32 WROOM (dev) , BOARDNAME = esp32 , edit the following const:
//static const uint8_t SS    = 15;
//static const uint8_t MOSI  = 13;
//static const uint8_t MISO  = 12;
//static const uint8_t SCK   = 14;
//You can get away with software SPI for maybe OLED, but TFT refresh will look awful


#ifdef SSD1306
  //Please read info above regarding SPI
  const byte SCREEN_WIDTH = 128; // OLED display width, in pixels
  const byte SCREEN_HEIGHT = 64; // OLED display height, in pixels
  const byte OLED_MOSI_PIN = 13;
  const byte OLED_CLK_PIN = 14;
  const byte OLED_DC_PIN = 26;
  const byte OLED_CS_PIN = 15;
  const byte OLED_RESET_PIN = 27;
  Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &SPI, OLED_DC_PIN, OLED_RESET_PIN, OLED_CS_PIN);
  //Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, OLED_MOSI_PIN, OLED_CLK_PIN, OLED_DC_PIN, OLED_RESET_PIN, OLED_CS_PIN);
#endif

#ifdef SSD1331
  //Please read info above regarding SPI
  // Color definitions
  #define  BLACK           0x0000
  #define BLUE            0x001F
  #define RED             0xF800
  #define GREEN           0x07E0
  #define CYAN            0x07FF
  #define MAGENTA         0xF81F
  #define YELLOW          0xFFE0  
  #define WHITE           0xFFFF
  const byte OLED_MOSI_PIN = 13;
  const byte OLED_CLK_PIN = 14;
  const byte OLED_DC_PIN = 26;
  const byte OLED_CS_PIN = 15;
  const byte OLED_RESET_PIN = 27;
  SSD_13XX oled = SSD_13XX(OLED_CS_PIN, OLED_DC_PIN, OLED_RESET_PIN);
#endif

#ifdef ST7735
  // Please read info above regarding SPI
  const byte TFT_CS_PIN = 15; // Hallowing display control pins: chip select
  const byte TFT_RST_PIN = 27; // Display reset
  const byte TFT_DC_PIN = 26; // Display data/command select
  const byte TFT_BACKLIGHT_PIN = 25; // Display backlight pin
  const byte TFT_MOSI_PIN = 13;  // Data out
  const byte TFT_SCLK_PIN = 14;  // Clock out
  const byte TFT_DIM_BRIGHTNESS = 1; // ~0.5%. drops consumption A LOT (~30mA-40mA).
  const byte TFT_FULL_BRIGHTNESS = 255; //100%. No real need for 100% backlight. Try lower values to save some juice.
  Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS_PIN, TFT_DC_PIN, TFT_RST_PIN);
  //Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS_PIN, TFT_DC_PIN, TFT_MOSI_PIN, TFT_SCLK_PIN, TFT_RST_PIN);
#endif

#ifdef LEDSEGMENT
  const byte LED_DIN_PIN = 23;
  const byte LED_CLK_PIN = 18;
  const byte LED_CS_PIN = 5;
  const byte LED_COUNT = 5;
  const byte LED_ADDR = 0;
  const byte LED_BRIGHTNESS_HIGH = 15; //0-15
  const byte LED_BRIGHTNESS_LOW = 1;
  LedControl lc=LedControl(LED_DIN_PIN,LED_CLK_PIN,LED_CS_PIN,LED_COUNT);
#endif




////////// 1.4.SCALE //////////
#ifdef PRO
  const byte ADC_LDO_EN_PIN = 21;
  const int ADC_LDO_ENABLE = 1; //high or low for enable ??? Check datasheet of your LDO.
  const int ADC_LDO_DISABLE = 0; //high or low for disable ??? Check datasheet of your LDO.
  
  const byte ADC_PDWN_PIN = 5;
  const byte ADC_DOUT_PIN = 19;
  const byte ADC_SCLK_PIN = 18;
  const byte ADC_GAIN0_PIN = 16;
  const byte ADC_GAIN1_PIN = 17;
  const byte ADC_SPEED_PIN = 22;
#endif

#ifdef LUNAR_ECLIPSE
  const byte ADC_PDWN_PIN = 4;
  const byte ADC_DOUT_PIN = 16;
  const byte ADC_SCLK_PIN = 17;
  const byte ADC_GAIN0_PIN = 12;
  const byte ADC_GAIN1_PIN = 13;
  const byte ADC_SPEED_PIN = 22;
#endif
const byte ADC_A0_PIN = 0; //tied to GND = AIN1 in PRO
const byte ADC_TEMP_PIN = 0; //tied to GND = AIN1 in PRO

SCALE scale = SCALE(ADC_PDWN_PIN, ADC_SCLK_PIN, ADC_DOUT_PIN, ADC_A0_PIN, ADC_SPEED_PIN, ADC_GAIN1_PIN, ADC_GAIN0_PIN, ADC_TEMP_PIN);
//SCALE scale = SCALE(ADC_PDWN_PIN, ADC_SCLK_PIN, ADC_DOUT_PIN);

const byte INITIALISING_SECS = 0; //how many seconds we will tare and wait. This is an extra settling time during boot.


////////// 1.5.BUTTONS //////////
//Note: T5 (IO12) is MTDI (strapping pin), MCU reads it on bootup. Be carefull.

//uncomment if you want to use the capacitive sensor of esp32
//#define TOUCH
//uncomment if you want to use any digital button (or external capacitive module with digital output)
#define BUTTON
#define INPUT_RESISTOR INPUT_PULLDOWN //standard button to IO/GND = INPUT_PULLUP , standard button to IO/VCC = INPUT_PULLDOWN. If you have external touch module, check its datasheet.
#define INTERRUPT_MODE RISING //INPUT_PULLUP -> FALLING or LOW, INPUT_PULLDOWN -> RISING OR HIGH


#ifdef TOUCH
// Larger value, more sensitivity
  const byte TOUCH_POWER_THRESHOLD = 10;
  const byte TOUCH_TARE_THRESHOLD = 10;
  const byte TOUCH_SECONDARY1_THRESHOLD = 10;
#endif

#ifdef PRO
// middle pin of CN6 is GPIO12 or T5
// middle pin of CN7 is GPIO33 or T8
// middle pin of CN8 is GPIO32 or T9
  const byte POWER_BUTTON_PIN = 32;
  const byte TARE_BUTTON_PIN = 12;
  const byte SECONDARY1_BUTTON_PIN = 33;
  #ifdef BUTTON
    //also, for wake up function we need to declare them as such (any other way ??? )
    //we only want to wake up if user touches power button
    #define POWER_BUTTON_PIN_RTC GPIO_NUM_32  
  #endif
#endif


#ifdef LUNAR_ECLIPSE
  const byte POWER_BUTTON_PIN = 32;
  const byte TARE_BUTTON_PIN = 33;
  const byte SECONDARY1_BUTTON_PIN = 14;
  #ifdef BUTTON
    //also, for wake up function we need to declare them as such (any other way ??? )
    //we only want to wake up if user touches power button
    #define POWER_BUTTON_PIN_RTC GPIO_NUM_32  
  #endif
#endif


////////// 1.5.OTHER //////////
uint32_t lastActionMillis = 0; 
bool snooze = false;
bool lightSleep = false;
bool wakeup = false;

bool initialising = false;
bool beginTare = false;
bool calibrating = false;
float calibrateToUnits = 0.0;
