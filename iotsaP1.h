#ifndef _IOTSAP1_H_
#define _IOTSAP1_H_
#include "iotsa.h"
#ifdef ESP32
#include <HardwareSerial.h>
#else
#include <SoftwareSerial.h>
#endif
#ifdef IOTSA_WITH_BLE
#include "iotsaBLEServer.h"
#include "iotsaBattery.h"
extern IotsaBatteryMod batteryMod;
#endif

#define MAX_TELEGRAM_SIZE 2048  // Maximum size of a data "telegram"

// P1 telegram parser class.
class P1Parser {
public:
  P1Parser(String& _telegram) : telegram(_telegram) {}
  P1Parser(const char * _telegram) : telegram(_telegram) {}
  bool valid();
  bool more();
  bool next(String& name, String& value);

private:
  String telegram;
};

// Pure telemetry module: no config, so no REST/IotsaApiProvider surface.
// Readings are available as plain (non-API) web endpoints and, on ESP32, BLE.
// IotsaBaseModule already is-a IotsaBLEProvider, so no extra BLE base needed.
class IotsaP1Mod : public IotsaBaseModule {
public:
  IotsaP1Mod(IotsaApplication &_app)
  : IotsaBaseModule(_app)
  {}
  void setup() override;
  void lateSetup() override;
  void loop() override;
  String info() override;
private:
  void handlerText();
  void handlerJson();
  void handlerXml();
  bool readTelegram();
  char telegram[MAX_TELEGRAM_SIZE];
  int telegramSize;
protected:
#ifdef IOTSA_WITH_BLE
  IotsaBleApiService bleApi;
  bool blePutHandler(UUIDstring charUUID) override { return false; };
  bool bleGetHandler(UUIDstring charUUID) override;
  static constexpr UUIDstring serviceUUID = "2B000000-BAAD-4A33-898A-3E8902CC1E7A";
  static constexpr UUIDstring p1UUID = "2B000001-BAAD-4A33-898A-3E8902CC1E7A";
#endif // IOTSA_WITH_BLE

};

#endif
