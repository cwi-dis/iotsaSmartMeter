//
// Read current electricity and gas usage from a smart meter adhering to the dutch
// P1 standard.
//
// All dutch smart meters *must* include a P1 port that allows you to read them out using
// a slightly convoluted serial protocol. The details of the P1 port can be found at
// http://files.domoticaforum.eu/uploads/Smartmetering/DSMR%20v4.0%20final%20P1.pdf
//
// Unfortunately, after some experiments, it turns out the P1 port doesn't supply enough
// power to start (while the base station hasn't been discovered the esp uses 170 mA or more).
// Project is on hold.
//

#include "iotsa.h"
#include "iotsaWifi.h"
#include <SoftwareSerial.h>

// CHANGE: Add application includes and declarations here

#define WITH_OTA    // Enable Over The Air updates from ArduinoIDE. Needs at least 1MB flash.

IotsaApplication application("Iotsa Smart Meter Server");
IotsaWifiMod wifiMod(application);

#ifdef WITH_OTA
#include "iotsaOta.h"
IotsaOtaMod otaMod(application);
#endif

//
// P1 interface.
//
#define PIN_ENABLE 13   // GPIO13 is ENABLE, active high, with external pullup to 5V (so we use pinMode tricks to drive it)
#define PIN_DATA 12     // GPIO12 is DATA, 115200 baud serial with inverted logic, internal pullup.
#define DATA_BAUDRATE 115200
#define MAX_TELEGRAM_SIZE 2048  // Maximum size of a data "telegram"

SoftwareSerial p1Serial(PIN_DATA, -1, true);

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

bool P1Parser::valid() {
  String hdr("/KFM5KAIFA-METER\r\n\r\n");
  if (telegram.startsWith(hdr)) {
    telegram.remove(0, hdr.length());
    return true;
  }
  return false;
}

bool P1Parser::more() {
  return !telegram.startsWith("!");
}

bool P1Parser::next(String& name, String& value) {
  int lParPos = telegram.indexOf('(');
  int lfPos = telegram.indexOf('\n');
  if (lfPos < 2 || lParPos < 0 || lParPos > lfPos) {
    name = "syntaxErrorAt";
    value = telegram;
    telegram = "!";
    return true;
  }
  name = telegram.substring(0, lParPos);
  value = telegram.substring(lParPos, lfPos-1);
  telegram.remove(0, lfPos+1);
  
  // See if we can parse the value
  if (name == "0-0:1.0.0") {
    name = "timestamp";
    value.remove(value.length()-1);
    value.remove(0,1);
    value = "20" + value.substring(0, 2) + "-" + value.substring(2, 4) + "-" + value.substring(4, 6) 
      + "T" + value.substring(6, 8) + ":" + value.substring(8, 10) + ":" + value.substring(10, 12);
    return true;
  }
  if (name == "0-0:96.1.1") {
    name = "meter_id_electricity";
    value.remove(value.length()-1);
    value.remove(0,1);
    return true;
  }
  if (name == "0-1:96.1.0") {
    name = "meter_id_gas";
    value.remove(value.length()-1);
    value.remove(0,1);
    return true;
  }

  if (name == "0-0:96.14.0") {
    name = "current_tariff";
    value.remove(value.length()-1);
    value.remove(0,1);
    return true;
  }
  if (name == "0-0:96.7.21") {
    name = "total_power_failures";
    value.remove(value.length()-1);
    value.remove(0,1);
    return true;
  }
  if (name == "0-0:96.7.9") {
    name = "total_power_failures_long";
    value.remove(value.length()-1);
    value.remove(0,1);
    return true;
  }

  if (name == "1-0:1.8.1") {
    name = "total_kwh_tariff_1";
    value.remove(11);
    value.remove(0,1);
    return true;
  }
  if (name == "1-0:1.8.2") {
    name = "total_kwh_tariff_2";
    value.remove(11);
    value.remove(0,1);
    return true;
  }
  if (name == "1-0:2.8.1") {
    name = "total_kwh_tariff_1_returned";
    value.remove(11);
    value.remove(0,1);
    return true;
  }
  if (name == "1-0:2.8.2") {
    name = "total_kwh_tariff_2_returned";
    value.remove(11);
    value.remove(0,1);
    return true;
  }
  
  if (name == "1-0:1.7.0") {
    name = "current_kw";
    value.remove(7);
    value.remove(0,1);
    return true;
  }
  if (name == "1-0:2.7.0") {
    name = "current_kw_return";
    value.remove(7);
    value.remove(0,1);
    return true;
  }
  
  if (name == "0-1:24.2.1") {
    name = "total_gas_m3";
    int pos = value.indexOf(')');
    value.remove(0, pos+1);
    value.remove(10);
    value.remove(0,1);
    return true;
  }

  return false;
}

// Declaration of the Hello module
class IotsaP1Mod : public IotsaMod {
public:
  IotsaP1Mod(IotsaApplication &_app) : IotsaMod(_app) {}
	void setup();
	void serverSetup();
	void loop();
  String info();
private:
  void handler();
  bool readTelegram();
  char telegram[MAX_TELEGRAM_SIZE];
  int telegramSize;
};


// Implementation of the Hello module
void IotsaP1Mod::setup() {
	pinMode(PIN_ENABLE, OUTPUT);  // Set ENABLE to output/low -> pulldown -> no enable signal
  digitalWrite(PIN_ENABLE, LOW);
  pinMode(PIN_DATA, INPUT_PULLUP);
  p1Serial.begin(DATA_BAUDRATE);
  p1Serial.setTimeout(3000);
}

bool IotsaP1Mod::readTelegram() {
  p1Serial.flush();
  pinMode(PIN_ENABLE, INPUT);  // Set ENABLE to input -> hi-Z -> enable signal via external pullup
  // Remove initial 0x00 byte
  telegramSize = p1Serial.readBytes(telegram, MAX_TELEGRAM_SIZE);
  pinMode(PIN_ENABLE, OUTPUT);  // Set ENABLE to output/low -> pulldown -> no enable signal
  while (telegramSize > 0 && telegram[0] != '/') {
    telegramSize--;
    memmove(telegram, telegram+1, telegramSize);
  }
  return telegramSize > 0;
}

void
IotsaP1Mod::handler() {
  // By default, return in the format acceptable to the first Accept: entry
  String format = server->header("Accept");
  int semiPos = format.indexOf(';');
  if (semiPos > 0) format.remove(semiPos);
  // Allow it to be overridden by format argument
  for (uint8_t i=0; i<server->args(); i++){
    if( server->argName(i) == "format") {
      format = server->arg(i);
    }
  }
  if (format == "" || format == "text/plain" || format == "*/*") {
    if (readTelegram()) {
      server->send_P(200, "text/plain", telegram, telegramSize);
    } else {
      server->send(503, "text/plain", "No P1 telegram received");
    }
  } else if (format == "json" || format == "application/json") {
    if (readTelegram()) {
      P1Parser p(telegram);
      if (p.valid()) {
        String message = "{";
        while(p.more()) {
          String name, value;
          p.next(name, value);
          message += "\"";
          message += name;
          message += "\":\"";
          message += value;
          message += "\"";
          if (p.more()) message += ",";
        }
        message += "}\n";
        server->send(200, "application/json", message);
      } else {
        String msg("Invalid P1 telegram received:\n");
        msg += telegram;
        server->send(503, "text/plain", msg);
      }
    } else {
      server->send(503, "text/plain", "No P1 telegram received");      
    }
  } else if (format == "xml" || format == "application/xml") {
    if (readTelegram()) {
      P1Parser p(telegram);
      if (p.valid()) {
        String message = "<smartMeter>";
        while(p.more()) {
          String name, value;
          bool known = p.next(name, value);
          if (known) {
            message += "<";
            message += name;
            message += ">";
            message += value;
            message += "</";
            message += name;
            message += ">";
          } else {
            message += "<unknown tag=\"";
            message += name;
            message += "\">";
            message += value;
            message += "</unknown>";
          }
        }
        message += "</smartMeter>\n";
        server->send(200, "application/xml", message);
      } else {
        server->send(503, "text/plain", "Invalid P1 telegram received");
      }
    } else {
      server->send(503, "text/plain", "No P1 telegram received");      
    }
  } else {
    String message = "Unknown format ";
    message += format;
    server->send(422, "text/plain", message);
  }
}

void IotsaP1Mod::serverSetup() {
  // Setup the web server hooks for this module.
  server->on("/p1", std::bind(&IotsaP1Mod::handler, this));
}

String IotsaP1Mod::info() {
  // Return some information about this module, for the main page of the web server.
  String rv = "<p>See <a href=\"/p1\">/p1</a> for current household energy usage";
  rv += " (Also available in <a href=\"/p1?format=json\">json</a> and <a href=\"/p1?format=xml\">xml</a>).";
  rv += "</p>";
  return rv;
}

void IotsaP1Mod::loop() {
  // Nothing to do in the loop, for this module
}

// Instantiate the Hello module, and install it in the framework
IotsaP1Mod p1Mod(application);

// Standard setup() method, hands off most work to the application framework
void setup(void){
#if 0
  // We lower power, the P1 port can only supply 100mA.
  WiFi.setOutputPower(0);
#endif

  application.setup();
  application.serverSetup();
  ESP.wdtEnable(WDTO_120MS);
}
 
// Standard loop() routine, hands off most work to the application framework
void loop(void){
  application.loop();
}

