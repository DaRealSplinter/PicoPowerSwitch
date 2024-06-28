#ifndef __SERVER
#define __SERVER

#include "defines.h"
#include "temperature.h"
#include "memory.h"
#include "gpio.h"
#include "screen.h"
#include "ethernetmodule.h"
#include "util.h"

class PowerServer {
public:
  PowerServer()
    : server(80){};
  void setup(Files* f);
  void loop(EthernetModule* ethernet, EEpromMemory* memory, Gpio* gpio, Temperature* temperature, Watchdog* watchdog);
private:
  Files* files;
  void sendPageBegin(HTMLBuilder* html, EthernetModule* ethernet, bool autoRefresh = false, int seconds = 10);
  void sendPageEnd(HTMLBuilder* html);
  void sendPowerPage(HTMLBuilder* html, EthernetModule* ethernet, Temperature* temperature, EEpromMemory* memory, Gpio* gpio, bool fastRefresh);
  void sendConfigPage(HTMLBuilder* html, EthernetModule* ethernet, EEpromMemory* memory);
  void sendConfigIPPage(HTMLBuilder* html, EthernetModule* ethernet, EEpromMemory* memory);
  void sendProcessHTMLPage(HTMLBuilder* html, EthernetModule* ethernet, String action, unsigned int timeout = 3);
  void sendProcessPage(HTMLBuilder* html, EthernetModule* ethernet, EEpromMemory* memory, char* action);
  void sendProcessIPPage(HTMLBuilder* html, EthernetModule* ethernet, EEpromMemory* memory, char* action);
  void sendServerPage(HTMLBuilder* html, EthernetModule* ethernet, EEpromMemory* memory, Gpio* gpio);
  void sendUploadPage(HTMLBuilder* html, EthernetModule* ethernet, String description = String(""));
  void sendErrorPage(HTMLBuilder* html, EthernetModule* ethernet);
  void sendFile(File* file);
  bool receiveFile(Watchdog* watchdog, File* file, unsigned int bytes);
  void processHeader(EthernetModule* ethernet, EEpromMemory* memory, Gpio* gpio, Temperature* temperature, char* action);
  void processPost(EthernetModule* ethernet, Watchdog* watchdog, char* action);
  EthernetServer server;
  EthernetClient client;
  String headerStringOn[NUM_DEVICES];
  String headerStringOff[NUM_DEVICES];
  String headerStringMinOn[NUM_DEVICES];
  String headerStringMinOff[NUM_DEVICES];
  String headerStringMinStat[NUM_DEVICES];
  String headerStringMain;
  String headerStringConfig;
  String headerStringProcess;
  String headerStringServer;
  String headerStringConfigIP;
  String headerStringProcessIP;
  String headerStringReboot;
  String headerStringUpload;
  String headerStringPostUpload;
  String headerStringUpgrade;
  String headerStringPostUpgrade;
};

#endif

