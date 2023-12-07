// CTRL - SHIFT - 'P' ; Upload data directory to little file system

#include "defines.h"
#include "util.h"
#include "serialport.h"
#include "memory.h"
#include "gpio.h"
#include "temperature.h"
#include "screen.h"
#include "ethernetmodule.h"
#include "powerserver.h"

#define APP_NAME "Pico Power Switch"

#define DEBUG
#define SERIALPORT
#define ETHERNET
#define ETHERNET_SERVER
#define GPIO
#define SCREEN
#define TEMPERATURE
#define MEMORY
#define BLINK
//#define CYLON
#ifndef CYLON
#define BACKUP_INDICATORS
#endif

Mutex wireMutex;
SerialPort port;
Blink blink;
Cylon cylon;
EEpromMemory memory(&wireMutex);
Temperature temperature;
Gpio gpio(&wireMutex);
Screen screen(&wireMutex);

Scan scanner(&wireMutex);
EthernetModule ethernet(&memory);
PowerServer powerServer;
Watchdog watchdog;
Files files;

Mutex startupMutex;
Mutex startupMutex1;

void setup() {
  //rp2040.enableDoubleResetBootloader();
  startupMutex.take();

  debugSetup();
  debugln();
  debugln("************************************************************");
  debugln(APP_NAME + String(" ") + String(PROGRAM_NUMBER) + String(".") + String(PROGRAM_VERSION_MAJOR) + String(".") + String(PROGRAM_VERSION_MINOR));

  blink.setup();
  port.setup(&scanner, &memory, &gpio, &screen, &temperature, &watchdog, &files);
  gpio.setup();
  memory.setup(&gpio);
  temperature.setup(&gpio, &memory);
  screen.setup(&gpio);
  scanner.setup();
  ethernet.setup(&gpio);
  powerServer.setup(&files);
  files.setup();
  cylon.setup(&gpio);
  watchdog.setup();

  startupMutex.give();
  startupMutex1.take();
  startupMutex1.give();

  gpio.setOnline(PICO, true);
  debugln(APP_NAME + String(" - Startup Complete"));
  debugln("************************************************************");
  port.complete();
}

void setup1() {
  startupMutex1.take();
  startupMutex1.give();

  startupMutex.take();
  startupMutex.give();
}

void loop() {
  blink.loop();
  ethernet.loop();
  powerServer.loop(&ethernet, &memory, &gpio, &temperature, &watchdog);
  screen.loop(APP_NAME, &temperature, &gpio, &ethernet);
  watchdog.loop();
  delay(1);
}

void loop1() {
  cylon.loop();
  port.loop();
  gpio.loop(&screen, &memory);
  memory.loop();
  temperature.loop();
  watchdog.loop();
  delay(1);
}
