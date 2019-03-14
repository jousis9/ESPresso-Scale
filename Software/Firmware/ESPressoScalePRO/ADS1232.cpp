/*  
  https://gitlab.com/jousis/ads1232-library
*/

#include "ADS1232.h"

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


ADS1232::ADS1232(byte pdwn, byte sclk, byte dout) //constructor
{   
  DEBUG_PRINTLN("Initializing ADC with minimal control");
  pdwnPin = pdwn;
  doutPin = dout;
  sclkPin = sclk;  
  a0Pin = 0;    
  spdPin = 0;
  gain1Pin = 0;
  gain0Pin = 0;
  tempPin = 0;

  initConfig();
} 

ADS1232::ADS1232(byte pdwn, byte sclk, byte dout, byte a0, byte spd, byte gain1, byte gain0, byte temp)
{
  DEBUG_PRINTLN("Initializing ADC with full control");
  pdwnPin = pdwn;
  doutPin = dout;
  sclkPin = sclk;
  spdPin = spd;
  gain1Pin = gain1;
  gain0Pin = gain0;
  a0Pin = a0;  
  tempPin = temp;

  initConfig();
}


void ADS1232::initConfig() {
  
  adcSpeed = 80;
  adcGain = 128;
  adcChannel = 0;
  adcTemp = 0;
  
  calFactor = 1400.0;
  tareOffset = 0.0;
  ignoreDiff = 0.0;
  ignoreDiffThreshold = 0.0;
  smoothing = true;
}

void ADS1232::begin() 
{
  pinMode(pdwnPin, OUTPUT);
  pinMode(doutPin, INPUT_PULLUP);
  pinMode(sclkPin, OUTPUT);

  powerOn();
  
  if (a0Pin >0) {
    pinMode(a0Pin, OUTPUT);
  }
  if (spdPin >0) {
    pinMode(spdPin, OUTPUT);
  }
  if (gain1Pin >0) {
    pinMode(gain1Pin, OUTPUT);
  }
  if (gain0Pin >0) {
    pinMode(gain0Pin, OUTPUT);
  }
  if (tempPin >0) {
    pinMode(tempPin, OUTPUT);
    digitalWrite(tempPin,LOW);
  }
  setGain(adcGain);
  setSpeed(adcSpeed);
  setChannel(adcChannel);
  delay(250);
  tare(false);
}





bool ADS1232::isReady()
{
  return digitalRead(doutPin) == LOW;
}

bool ADS1232::safeWait()
{
  //if you want to implement a more robust and sophisticated timeout, please check out:
  //https://github.com/HamidSaffari/ADS123X    
  uint32_t elapsed;
  elapsed = millis();
  while (!isReady()) {
    if (millis() > elapsed + 2000) {
      //timeout
      SERIAL_PRINTLN("Error while waiting for ADC");
      return false;
    }
  }
  return true;    
}

void ADS1232::powerOn()
{
  digitalWrite(pdwnPin, HIGH);
  digitalWrite(sclkPin, LOW);
//  while (!isReady()) { };  
  if (!safeWait()) {
    SERIAL_PRINTLN("Power on error");
    return;
  }
  calibrateADC();
}

void ADS1232::powerOff()
{
  digitalWrite(pdwnPin, LOW);
  digitalWrite(sclkPin, HIGH);
}

void ADS1232::setGain(uint8_t gain) // 1/2/64/128 , default 128
{
  byte adcGain1;
  byte adcGain0;
  if(gain == 1) {
    adcGain1 = 0;
    adcGain0 = 0;
  } else if (gain == 2) {
    adcGain1 = 0;
    adcGain0 = 1;
  } else if (gain == 64) {
    adcGain1 = 1;
    adcGain0 = 0;
  } else {
    //default == 128
    adcGain1 = 1;
    adcGain0 = 1;
  }
  if (gain0Pin>0 && gain1Pin>0) {
    digitalWrite(gain1Pin,adcGain1);
    digitalWrite(gain0Pin,adcGain0);
  }
  calibrateADC();
}


void ADS1232::setSpeed(byte sps) //10 or 80 sps , default 10
{  
  adcSpeed = sps;    
  if (spdPin > 0 ) {
    if(adcSpeed == 80) {
      digitalWrite(spdPin,HIGH);
    } else {
      digitalWrite(spdPin,LOW);
    }    
  }
  calibrateADC();
}

byte ADS1232::getSpeed() 
{  
  return adcSpeed;
}

void ADS1232::setChannel(byte channel) //0 or 1
{  
  if (a0Pin > 0 ) {
      digitalWrite(a0Pin,channel);
  }
  calibrateADC();
}

void ADS1232::setDataSetSize(byte datasetsize) 
{
  if (datasetsize != DATA_SET) {
    DATA_SET = datasetsize;  
    resetSmoothing(0);    
    DEBUG_PRINT("dataset size changed to ");DEBUG_PRINTLN(DATA_SET);
  }
}

void ADS1232::setSmoothing(bool enable) {
  smoothing = enable;
  resetSmoothing(0);
}
bool ADS1232::getSmoothing() {
  return smoothing;
}

void ADS1232::resetSmoothing(int32_t value) {
  if (DATA_SET > DATA_SET_MAX) {
    DATA_SET = DATA_SET_MAX;
  } else if (DATA_SET < DATA_SET_MIN) {
    DATA_SET = DATA_SET_MIN;
  }  
  if (value == 0) {
    //zero is our flag for current value
    value = readADC();
  }
  readIndex = 0;
  for (byte i = 0; i <= DATA_SET_MAX + 1; i++) {
    dataSampleSet[i]=value;
  }
}

//void ADS1232::readInternalTemp() 
//{  
//  //Not implemented.
//  //If you need to, it is not very hard, check datasheet
//}


void ADS1232::calibrateADC() 
{  
  DEBUG_PRINTLN("ADC calibration init...");
  readADC();
  //readADC returns with 25th pulse completed, so immediately continue with 26th
  delayMicroseconds(1);
  digitalWrite(sclkPin, HIGH);  // 26th pulse
  delayMicroseconds(1);  
  digitalWrite(sclkPin, LOW);  // end of 26th
  //actual calibration begins... wait for dout = LOW
  if (!safeWait()) {
    //oops...time out !!!
    SERIAL_PRINTLN("ADC calibration error");
    return;
  }
  DEBUG_PRINTLN("ADC calibration done...");
  //all done, ready to read again...
}

void ADS1232::tare(bool quick)
{  
  if (quick) {
    tareOffset = lastAdcValue;
  } else {
    tareOffset = readADC();    
  }
  resetSmoothing(tareOffset);
}

void ADS1232::setCalFactor(float cal) 
{
  if (cal > 0) {
    calFactor = cal;    
  } else {
    calFactor = 1.0;
  }
}

void ADS1232::setMinDiff(int32_t diff, int32_t threshold) 
{
  //changes less than abs(diff) when value < tareOffset + threshold will not be accounted.
  //useful when near 0grams, seems more accurate to the user :D 
  ignoreDiff = diff;
  ignoreDiffThreshold = threshold;
}


double ADS1232::readUnits(byte samples) {
  double unitsvalue = 0.0;
  unitsvalue = (double)(readRaw(samples)-tareOffset)/(double)calFactor;
  //Can we go down to 0.001 range or more???
  //Sure, I doubt though it can be useful with 3.3V excitation and normal off the shelf load cells.
  //In that case, do we really need double here? No. You can change it to float.
  
  //Remember that Serial cannot print more than 2 decimal digits.  
  //Uncomment the following for testing
  //unitsvalue = unitsvalue*100.0;  
  //DEBUG_PRINT("Super duper resolution value*100 = ");DEBUG_PRINTLN(unitsvalue);
  if (unitsvalue == -0.00) { unitsvalue = 0.0; }
  return unitsvalue;
}


int32_t ADS1232::getSmoothedValue()
{
  //simple h/l rejection algorithm from the following library
  //https://github.com/olkal/HX711_ADC
  int32_t data = 0;
  int32_t L = 2147483647;
  int32_t H = -2147483648;
  for (int r = 0; r < DATA_SET; r++) {
    if (L > dataSampleSet[r]) L = dataSampleSet[r]; // find lowest value
    if (H < dataSampleSet[r]) H = dataSampleSet[r]; // find highest value
    data += dataSampleSet[r];
  }
  data -= L; //remove lowest value
  data -= H; //remove highest value
  //Uncomment the following to debug your load cell
  //DEBUG_PRINT(" L / H "); DEBUG_PRINT(L);DEBUG_PRINT("/");DEBUG_PRINTLN(H);
  return data/(DATA_SET-2);
}


int32_t ADS1232::readRaw(byte samples)
{
  int32_t valuessum=0;
  for (byte i=0;i<samples;i++){
    valuessum += readADC();  
  }

  lastAdcValue = valuessum/samples;  
  if (!smoothing) {
    //lastAdcValue = valuessum/samples;    
  } else {

    //In any case, put the value to the array
    if (DATA_SET > DATA_SET_MAX) {
      DATA_SET = DATA_SET_MAX;
      resetSmoothing(0);
    } else if (DATA_SET < DATA_SET_MIN) {
      DATA_SET = DATA_SET_MIN;
      resetSmoothing(0);
    }
    if (readIndex > DATA_SET - 1) {
      readIndex = 0;
    } else {
      readIndex++;
    }
    dataSampleSet[readIndex] = lastAdcValue;


    
    lastAdcValue = getSmoothedValue();       
  }
  

  readsPerSecond++;
  if (millis() - lastRateCheck > 1000) {
    actualSPS = readsPerSecond*1000/(millis()-lastRateCheck);
    readsPerSecond = 0;
    lastRateCheck = millis();
  }
  
  return lastAdcValue;
}




int32_t ADS1232::readADC()
{
  
  int32_t adcvalue = 0;

  if (!safeWait()) {
    return 0;
  }
  //ADC ready...begin

  //manual 1-bit read
  //for esp32 this is a more reliable method than shiftin since we can add a slight delay and avoid spikes in read values  
  //each pulse must be at least 100ns = 0.1μs. We insert a delay here of 1μs to be sure
  //alternatively you can use 8bit shiftin read => adcvalue = shiftIn(doutPin, sclkPin, MSBFIRST); adcvalue <<= 8; , etc...
  int i=0;
  for(i=0; i < 24; i++) { //24 bits => 24 pulses
    digitalWrite(sclkPin, HIGH);
    delayMicroseconds(1);
    adcvalue = (adcvalue << 1) + digitalRead(doutPin);
    digitalWrite(sclkPin, LOW);
    delayMicroseconds(1);
  }  
  adcvalue = (adcvalue << 8) / 256;  
  digitalWrite(sclkPin, HIGH); // keep dout high // 25th pulse
  delayMicroseconds(1);
  digitalWrite(sclkPin, LOW); 
  //wait for it to become high actually
  while(digitalRead(doutPin) != HIGH)
  {
    yield();
//      DEBUG_PRINTLN("waiting for dout");
  }

  return adcvalue;
}
