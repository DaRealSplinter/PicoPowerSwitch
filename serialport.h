#include "util.h"
#include "memory.h"
#include "gpio.h"
#include "screen.h"

#ifndef __SERIAL
#define __SERIAL

typedef enum {
  TRACE,
  INFO,
  WARNING,
  ERROR,
  HELP,
  PASSED,
  FAILED,
  PROMPT
} PRINT_TYPES;

typedef enum {
  Normal = 0,
  Black = 30,
  Red,
  Green,
  Yellow,
  Blue,
  Magenta,
  Cyan,
  White
} COLOR;


void debugSetup();
void println();
void print(PRINT_TYPES type, String line);
void print(PRINT_TYPES type, String line, String line2);
void println(PRINT_TYPES type, String line);
void println(PRINT_TYPES type, String line, String line2);

class SerialPort {
public:
  SerialPort(){};
  void setup(Scan* scan, EEpromMemory* eepromMemory, Gpio* gpioControl, Screen* oledscreen, Temperature* tempStatus, Watchdog* watchdog, Files* files);
  void loop();
  void complete();
private:
};

#endif
