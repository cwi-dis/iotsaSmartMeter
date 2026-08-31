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

IotsaApplication application("Iotsa Smart Meter Server");
IotsaWifiMod wifiMod(application);

#include "iotsaOta.h"
IotsaOtaMod otaMod(application);

#ifdef IOTSA_WITH_BLE
#include "iotsaBattery.h"
IotsaBLEServerMod bleserverMod(application);
IotsaBatteryMod batteryMod(application);
#endif

// Instantiate the P1 module, and install it in the framework
IotsaP1Mod p1Mod(application);

// Standard setup() method, hands off most work to the application framework
void setup(void){
  application.setup();
  application.lateSetup();
#ifndef ESP32
  ESP.wdtEnable(WDTO_120MS);
#endif
}

// Standard loop() routine, hands off most work to the application framework
void loop(void){
  application.loop();
}
