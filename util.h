#ifndef __UTIL
#define __UTIL

#include <FreeRTOS.h>
#include <semphr.h>
#include "LittleFS.h"

#define UPGRADE_FILE_NAME "pico.bin"
#define UPGRADE_COMMAND_FILE_NAME "otacommand.bin"

void UPGRADE_SYSTEM();

typedef enum {
  JAXSON,
  DRAGON,
  LIGHT,
  BITMAP_LENGTH
} BITMAP;

class Task {
public:
  Task()
    : refresh(millis()), timeout(100){};
  void setRefresh(unsigned long time) {
    timeout = time;
  };
  bool run();
  void reset();
private:
  unsigned long refresh;
  unsigned long timeout;
};

class Mutex {
public:
  Mutex();
  void take();
  void give();
private:
  SemaphoreHandle_t mutex;
};

class Watchdog : protected Task {
public:
  Watchdog(uint32_t timeout = 8300, uint32_t petCycle = 1000)
    : watchdogTimeout(timeout), watchdogPetCycle(petCycle), resetFlag(true){};
  void setup();
  void loop();
  void petWatchdog();
  void reboot();
private:
  Mutex mutex;
  uint32_t watchdogTimeout;
  uint32_t watchdogPetCycle;
  bool resetFlag;
};

class Blink : protected Task {
public:
  Blink()
    : state(false){};
  void setup() {
    setRefresh(500);
    pinMode(LED_BUILTIN, OUTPUT);
  };
  void loop();
private:
  bool state;
};

class Files {
public:
  void setup();
  File getFile(String path);
  void deleteFile(String path);
  File writeFile(String path);
  void printInfo();
  unsigned int availableSpace();
};

class DebugLED {
public:
  virtual bool getDebug(int pin) = 0;
  virtual void setDebug(int pin, bool state = true) = 0;
  virtual void clearDebug() = 0;
};

class Output {
public:
  virtual void setScreen(String line1 = "", String line2 = "", String line3 = "", String line4 = "", String line5 = "", String line6 = "", String line7 = "", String line8 = "") = 0;
  virtual void setScreen(BITMAP bitmap) = 0;
  virtual void setScreen(BITMAP bitmap, String caption) = 0;
};

class Data {
public:
  virtual char* getDeviceName(byte device) = 0;
};

class HTMLBuilder {
public:
  HTMLBuilder();
  void print(const char* line);
  void println(const char* line);
  void println();
  void print(String line);
  void println(String line);
  void print(int line);
  void println(int line);
  char* buffer();
  unsigned int length();
private:
  unsigned int index;
};

class Cylon : protected Task {
public:
  Cylon()
    : state(-1){};
  void setup(DebugLED* dbl) {
    setRefresh(250);
    _dbl = dbl;
    _dbl->clearDebug();
  };
  void loop();
private:
  DebugLED* _dbl;
  int state;
};

class Scan {
public:
  Scan(Mutex* wireMutex)
    : mutex(wireMutex){};
  void setup();
  void scan(Output* screen);
private:
  Mutex* mutex;
};

#endif
