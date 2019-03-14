/*  
  https://gitlab.com/jousis/espresso-scale
*/

#include "SCALE.h"

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



SCALE::SCALE(byte pdwn, byte sclk, byte dout) : adc( new ADS1232(pdwn, sclk, dout))
{
  
} 

SCALE::SCALE(byte pdwn, byte sclk, byte dout, byte a0, byte spd, byte gain1, byte gain0, byte temp) : adc ( new ADS1232(pdwn, sclk, dout, a0, spd, gain1, gain0, temp))
{
  
}



void SCALE::begin(byte quick) 
{
  adc->begin();
  
  if (quick != 1) {
    int i=0;
    for (i=0;i<10;i++){ // do 10 tares and wait 5s for settling
      tare();
      delay(100);
    }
  }
}




void SCALE::powerOn()
{
  adc->powerOn();
}

void SCALE::powerOff()
{
  adc->powerOff();
}


void SCALE::setSpeed(byte sps) //10 or 80 sps , default 10
{
  adc->setSpeed(sps);
}
byte SCALE::getSpeed()
{
  return(adc->getSpeed());
}


int SCALE::getAdcActualSPS()
{  
  return adc->actualSPS;
}

void SCALE::setGain(byte gain)
{  
  adc->setGain(gain);
}

void SCALE::setScaleMode(byte scalemode) {
  scaleMode=scalemode;
  switch (scalemode) {
    case 1 :
      autoTareWhenStable = true;
      break;
    default :
      autoTareWhenStable = false;    
  }
}

void SCALE::setSensitivity(byte sensitivity)
{
  int datasetsize = 1;
  if (sensitivity <= 0) { sensitivity = 1; }
  datasetsize = (int) DATA_SET_MAX/sensitivity;
  adc->setDataSetSize(datasetsize);
}

void SCALE::setSmoothing(byte smoothing) {
    adc->setSmoothing(smoothing == 1);    
}

byte SCALE::getSmoothing() {
  if (adc->getSmoothing()) {
    return 1;    
  } else {
    return 0;
  }
}


void SCALE::calibrateADC() 
{  
  adc->calibrateADC();
}

void SCALE::calibrate(float targetWeight, int maxMillis, float targetDiff) 
{  
  tare();
  uint32_t calibrationStartTime = millis();
  float weight = 0.0;
  float oldstableWeightDiff = stableWeightDiff;
  //increase stableWeightDiff to 1g  
  stableWeightDiff = 1.0;
  //loop until settled
  uint32_t elapsedTime = millis() - calibrationStartTime;  
  while (!hasSettled || weight < 10.0) {
    weight = readUnits(1);    
    elapsedTime = millis() - calibrationStartTime;
    if (elapsedTime > maxMillis) {
      DEBUG_PRINTLN("calibration failed");
      return;
    }
  }
  
  float switchModeThreshold = targetWeight*0.05; //5%
  bool initAutoCalibrationComplete = false;
  bool fineTuneAutoCalibrationComplete = false;
  bool slowTuneAutoCalibrationComplete = false;
  byte oldSpeed = adc->getSpeed();
  bool oldSmoothing = adc->getSmoothing();
  adc->setSmoothing(false);  
  adc->setSpeed(80);
  //let's get close enough very fast
  float newCalFactor = adc->calFactor;
  while (!initAutoCalibrationComplete && elapsedTime<maxMillis) {
    weight = readUnits(1);
    if ( fabs(weight-targetWeight) <= switchModeThreshold) {
      //continue in low speed
      initAutoCalibrationComplete = true;
    } else {
      if (weight > targetWeight) {
        //increase calfactor
        newCalFactor = newCalFactor + 50;
      } else {
        //decrease calfactor
        newCalFactor = newCalFactor - 50;
      }    
      adc->setCalFactor(newCalFactor);
      DEBUG_PRINT("new calfactor/weight ");DEBUG_PRINT(newCalFactor);DEBUG_PRINT("/");DEBUG_PRINTLN(weight);
    }
    elapsedTime = millis() - calibrationStartTime;  
  }
  DEBUG_PRINTLN("Fast calibration completed...");
  switchModeThreshold = targetWeight*0.01; //1%
  
  while (!fineTuneAutoCalibrationComplete && elapsedTime<maxMillis) {
    weight = readUnits(1);
    if ( fabs(weight-targetWeight) <= switchModeThreshold) {
      fineTuneAutoCalibrationComplete = true;
    } else {
      if (weight > targetWeight) {
        //increase calfactor
        newCalFactor = newCalFactor + 1;
      } else {
        //decrease calfactor
        newCalFactor = newCalFactor - 1;
      }    
      adc->setCalFactor(newCalFactor);
      DEBUG_PRINT("new calfactor/weight ");DEBUG_PRINT(newCalFactor);DEBUG_PRINT("/");DEBUG_PRINTLN(weight);
    }
    elapsedTime = millis() - calibrationStartTime;  
  }
  stableWeightDiff = oldstableWeightDiff;

  bool above = false;
  bool below = false;
  //finally, switch to 10SPS and do the final approach
  DEBUG_PRINTLN("swithing to 10SPS");
  adc->setSpeed(10);
  bool resetStableWeightCounter = true;
  while (!slowTuneAutoCalibrationComplete && elapsedTime<maxMillis) {
    weight = readUnits(1);
    if ( (fabs(weight-targetWeight) <= targetDiff) && above && below) {
      //almost done...wait until settled
      if (resetStableWeightCounter) {
        hasSettled = false;
        resetStableWeightCounter = false;
        stableWeightCounter = 0;
        DEBUG_PRINTLN("Almost there...reseting hasSettled status");
      }
      if (hasSettled) { slowTuneAutoCalibrationComplete = true; }
    } else {
      if (weight > targetWeight) {
        //increase calfactor
        above = true;
        newCalFactor = newCalFactor + finetuneCalibrationAdj;
      } else {
        //decrease calfactor
        below=true;
        newCalFactor = newCalFactor - finetuneCalibrationAdj;
      }    
      adc->setCalFactor(newCalFactor);
      DEBUG_PRINT("new calfactor/weight ");DEBUG_PRINT(newCalFactor);DEBUG_PRINT("/");DEBUG_PRINTLN(weight);
    }
    elapsedTime = millis() - calibrationStartTime;  
  }

  if (elapsedTime>=maxMillis){
    DEBUG_PRINTLN("calibration timed out...please increase time");
  } else {
    DEBUG_PRINTLN("final calibration completed...");
  }
  adc->setSpeed(oldSpeed);  
  adc->setSmoothing(oldSmoothing);  
  DEBUG_PRINTLN("DONE");
}

void SCALE::setCalFactor(float calFactor)
{
  adc->setCalFactor(calFactor);
}
float SCALE::getCalFactor()
{
  return(adc->calFactor);
}

void SCALE::tare(bool quick) 
{  
  adc->tare(quick);
  lastStableWeight = 0;
}

double SCALE::roundToDecimal(double value, int dec)
{
  double mlt = powf( 10.0f, dec );
  value = roundf( value * mlt ) / mlt;
  return value;
}

double SCALE::readUnits(byte samples)
{
  double returnunits = 0.0;
  double units = 0.0;
  if (samples <=0) {
    samples=1;
  }
  units = adc->readUnits(samples);
  int samplesize = adc->getSpeed()*stableWeightSampleSizeMultiplier;
  if (fabs(fabs(units) - fabs(lastUnitRead)) <= stableWeightDiff) {
    //ok...stable weight, increase counter and change hasSettled flag
    if (stableWeightCounter < samplesize) { 
      stableWeightCounter++;
    }
    if (stableWeightCounter >= samplesize) {
      if (!hasSettled) {
        //1st time
        hasSettled = true;
        lastStableWeight = units;        
      }
    }
  } else {
    //oops...
    hasSettled = false;
    lastStableWeight = units + fakeStabilityRange*2;
    stableWeightCounter=0;
  }


  uint32_t rocMillis = millis();
  if (rocMillis > rocLastCheck + rocInterval) {
    roc = (units - lastUnitRead)*1000/(rocMillis - rocLastCheck);
    rocLastCheck = rocMillis + rocInterval;
  }
  if (hasSettled && fabs(units) > 0.01 && (fabs(units) < zeroTracking)){
    tare();
    units = 0.0;
  }
  
  if (hasSettled && autoTareWhenStable && fabs(units) > 0) {
    if (lastUnitRead > 0) {
      //only store last value if > 0
      lastTareWeight = lastUnitRead;
      lastTareWeightRounded = roundToDecimal(lastUnitRead,decimalDigits);
    }
    tare();
    units = 0.0;
  }

  returnunits = units;
  if (fakeStabilityRange > 0) {
    if (fabs(fabs(units) - fabs(lastFakeRead)) <= fakeStabilityRange) {
      if ((millis() - lastFakeRefresh) < fakeDisplayLimit*1000){
        //we are fine...show the fake
        returnunits = lastFakeRead;
      } else {
        //nope...not any more
        lastFakeRead = units;
        lastFakeRefresh = millis();
      }
    } else {
      lastFakeRead = units;
      lastFakeRefresh = millis();
    }
  }
  lastUnitRead = units;
  
//  if (fabs(units) <= zeroRange) { units = 0.00; }  
  if (fabs(roc) <= zeroRange) { roc = 0.00; }
//  if (fabs(fabs(units)-fabs(lastStableWeight)) <= fakeStabilityRange) { units = lastStableWeight; }
  if (fabs(returnunits) <= zeroRange) { returnunits = 0.00; }
  return roundToDecimal(returnunits,decimalDigits);
}
