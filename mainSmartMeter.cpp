//
// Read current electricity and gas usage from a smart meter adhering to the dutch
// P1 standard.
//
// All dutch smart meters *must* include a P1 port that allows you to read them out using
// a slightly convoluted serial protocol. The details of the P1 port can be found at
// http://files.domoticaforum.eu/uploads/Smartmetering/DSMR%20v4.0%20final%20P1.pdf
//

#include "iotsa.h"
#include "iotsaWifi.h"
#include "iotsaP1.h"

#define WITH_OTA    // Enable Over The Air updates from ArduinoIDE. Needs at least 1MB flash.

IotsaApplication application("Iotsa Smart Meter Server");
IotsaWifiMod wifiMod(application);

#ifdef WITH_OTA
#include "iotsaOta.h"
IotsaOtaMod otaMod(application);
#endif

#ifdef IOTSA_WITH_BLE
#include "iotsaBattery.h"
IotsaBLEServerMod bleserverMod(application);
IotsaBatteryMod batteryMod(application);
#endif

// Instantiate the P1 module, and install it in the framework
IotsaP1Mod p1Mod(application);

// Standard setup() method, hands off most work to the application framework
void setup(void){
#if 0
  // We lower power, the P1 port can only supply 100mA.
  WiFi.setOutputPower(0);
#endif

  application.setup();
  application.serverSetup();
#ifndef ESP32
  ESP.wdtEnable(WDTO_120MS);
#endif
}

// Standard loop() routine, hands off most work to the application framework
void loop(void){
  application.loop();
}
