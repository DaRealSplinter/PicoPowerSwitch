#include "util.h"
#include "memory.h"
#include "gpio.h"
#include "screen.h"

#ifndef __DEBUG
#define __DEBUG

void debugSetup();
void debug(String line);
void debugln(String line);

#endif

#ifndef __SERIAL
#define __SERIAL

class SerialPort {
public:
  SerialPort(){};
  void setup(Scan* scan, EEpromMemory* eepromMemory, Gpio* gpioControl, Screen* oledscreen, Temperature* tempStatus, Watchdog* watchdog, Files* files);
  void loop();
  void complete();
private:
};

#endif
