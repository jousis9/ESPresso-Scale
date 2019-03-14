/*  
  ESPresso Scale
  Created by John Sartzetakis, Jan 2019
  Released into the public domain.
  https://gitlab.com/jousis/espresso-scale
*/

#ifndef SCALE_h
#define SCALE_h

#include <Arduino.h>
#include "ADS1232.h"

class SCALE
{

  public:
    SCALE(byte pdwn, byte sclk, byte dout, byte a0, byte spd, byte gain1, byte gain0, byte temp); // constructor with full control
    SCALE(byte pdwn, byte sclk, byte dout);                                                       // basic constructor
    void begin(byte quick = 0);
    void setGain(byte gain = 10); 
    byte getSpeed();
    void setSpeed(byte sps = 10);                                                                    // 10 or 80 sps , default 10
    void powerOn(); 
    void powerOff(); //power off does put the ADC to sleep but does NOT shut down any LDO(s) you might have to power it up.
    void calibrateADC(); //this is the internal calibration method of the ADC , not the calculation of the calFactor. Invoked on powerOn() but you might want to manually run it if you detect erratic readings.
    void calibrate(float targetWeight, int maxMillis, float targetDiff); // this function will do a single point weight calibration
    void setCalFactor(float calFactor); // directly set your calfactor
    float getCalFactor();
    void tare(bool quick = true); //quick does not adds new read, just tares the last value.
    byte scaleMode = 0; //0=normal, 1=auto tare
    void setScaleMode(byte scalemode);
    double readUnits(byte samples);
    int getAdcActualSPS();

    //Smoothing enables/disables high/low rejection and averaging of samples
    //If disabled using 80SPS (not recommended for real world use) scale can detect 100g change on a 200g rated load cell within 4 samples , ~50ms (useless but impressive).
    //But, if your goal is to have a very stable low range scale with very sensitive load cell (ex 100g), disable smoothing and increase readSamples.
    void setSmoothing(byte smoothing);
    byte getSmoothing();
    
    //sensitivity < DATA_SET_MIN ==> maximizes samples (default is 20) ==> low responsiveness ==> don't do it for 10sps
    //sensitivity > DATA_SET_MAX ==> minimizes samples (default is 5) ==> high responsiveness ==> best for 10sps but try it for 80sps if you have a good load cell
    //keep in mind that even with huge dataset (100+ samples - see ADS1232.h) you cannot eliminate the 0.01 variation with 3.3V excitation. You only increase lag.
    //maximum sensitivity is optimized for 10SPS and minimum for 80SPS.  
    void setSensitivity(byte sensitivity); 
    

    //STABILITY PARAMETERS // VALUES DEPEND ON YOUR LOAD CELL/PCB/SPEED // USE WITH CAUTION
    //Please be carefull when choosing these values.
    //They will affect very much your resolution,response time, stability everywhere.
    //If you only want to stabilize the output measurements and not mess with the ADC options, check out the fake stability algorithm below.
    bool hasSettled = false; //is the last reading the final or is still increasing/decreasing ??
    int stableWeightCounter = 0;
    byte stableWeightSampleSizeMultiplier = 2; //2*SPS , ~2s. If you want faster auto tare or if your readings are erratic decrease it. If you want to slow down autotare decrease it but also check out stableWeightDiff.
    
    //the following option will also affect the sleep timer and the auto tare function of your scale. If you put 0.01 and your scale cannot stabilize within +/-0.01g, it will never sleep or auto tare.
    float stableWeightDiff = 0.05; // if last stableWeightSampleSize number of adc values(in units) are within +/-stableWeightDiff, hasSettled flag = true


    //in the final phase of calibration, we add/substract this amount from our calibration factor until we swing around our target weight.
    //If you insert here 10 and your calibration factor is ~1000 , your margin of error will be (max) 1%, which is a lot.
    //But if your (theoretical) target calibration factor is around 9827 and you put 0.2, the final approach might take a lot.
    //The calibration procedure goes as follows
    //Step 1-> generic approach using 80SPS => adding/subtracting 50 to calfactor => ends when our weight is within 5% of the target weight
    //Step 2-> refined approach using 80SPS => adding/subtracting 10 to calfactor => ends when our weight is within 1% of the target weight
    //Step 3-> final approach using 10SPS => adding/subtracting finetuneCalibrationAdj to calfactor => ends when our weight is within stableWeightDiff of the target weight
    float finetuneCalibrationAdj = 0.25;
    
    float zeroTracking = 0.05; //0=disabled, Auto tares scale when hasSettled and weight(grams) < zeroTracking    
    float zeroRange = 0.05; //if result (units) is within +/-zeroRange will be SHOWN as 0 (but not tared). It only affects the perceived stability around 0. Combine it with fakeStabilityRange for ultimate fakeness.



    // FAKE STABILITY PARAMETERS // Will NOT affect any useful function // Use without caution :)
    //Do not SHOW changes less than +/- fakeStabilityRange. This will not affect measurements. It will only make the user think that this scale is awesome(r).
    float fakeStabilityRange = 0.1; 
    byte fakeDisplayLimit = 1; //how many seconds at most we will show this fake number before resetting? 1s is fine, we will have jumps only 1/second.
    float lastFakeRead = 0;
    uint32_t lastFakeRefresh = 0; //millis
    float lastStableWeight = 0; //keeps the last settled weight. Will use it to display when in "fake stability" mode.
    ///////

    //If you don't want to annoy users with minor variations, consider using the fake stability algorithm above.
    //But, even if you change the decimal digits, you will not affect the calculations done on the ADC. Rounding only happens on the output.
    byte decimalDigits = 2; //decimal units (resolution) of the units returned


    //COFFEE FEATURES
    bool autoTareWhenStable = false; // automatically tares the scale after settling time (see stableWeightSampleSize).
    float lastTareWeight = 0; //keeps the last settled weight before (auto)taring.
    float lastTareWeightRounded = 0; //keeps the last settled weight before (auto)taring.

    
    

    //rate of change interval is the interval of roc calculation in ms.
    //if you have 80SPS and your interval is 1000ms you will never detect correctly rapid changes in weight.
    //On the other side, since roc is between 2 measurements only if you calculate it too frequent you will have very big values if your load cell is not so stable or if using 80SPS.
    //try different values and see. Best to keep it in line with SPS if you want to detect high speed roc => 100 for 10SPS , 12.5 for 80SPS
    float rocInterval = 100.0;
    float roc = 0.0; //rate of change , g per second
    uint32_t rocLastCheck = 0; // millis
  
  protected:
    ADS1232* adc;
    double roundToDecimal(double value, int dec);
    double lastUnitRead = 0;
};

#endif
