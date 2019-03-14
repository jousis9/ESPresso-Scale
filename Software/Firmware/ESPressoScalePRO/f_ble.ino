/* 
   \\\\\\\\\\\\\\\\\\\\    6.BLE    ////////////////////
*/

bool deviceConnected = false;
bool oldDeviceConnected = false;
uint32_t lastBleUpdate = 0;
BLEServer* pServer = NULL;
BLECharacteristic* pWeightCharacteristic = NULL; //READ + NOTIFY => returns weight in grams (string)
BLECharacteristic* pSettingsCharacteristic = NULL; //READ + WRITE

////////// 6.1.API //////////
String applyBLECommand(std::string bleInput) {
      String outputString = "ERROR_BLE";
      String bleInputString = bleInput.c_str();
      DEBUG_PRINT("settings write: ");
      DEBUG_PRINTLN(bleInputString);    

    int INPUT_SIZE = 30;
    char input[INPUT_SIZE + 1];

    bleInputString.toCharArray(input,INPUT_SIZE);
    
    char* separator = strchr(input, '=');    
    if (separator != 0)
    {
        *separator = 0;
        String command = input;
        ++separator;        
        outputString = handleCommand(command,separator);
       DEBUG_PRINTLN(outputString);
    }
    return outputString;
}


////////// 6.2.CALLBACKS //////////
class bleServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };
    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};


class bleSettingsCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      String output = applyBLECommand(pCharacteristic->getValue());
      pCharacteristic->setValue(output.c_str());
      DEBUG_PRINT("Returning value ");DEBUG_PRINTLN(output);
    }
    void onRead(BLECharacteristic *pCharacteristic) {
      DEBUG_PRINTLN("Settings READ");
    }
};
