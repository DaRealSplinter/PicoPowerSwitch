#ifndef __GPIO
#define __GPIO

#include <TCA9555.h>
#include "util.h"

#define PICO 0
#define SSD1306 1
#define BUTTON 2
#define DHT_11 3
#define W5500 4
#define I2C_EEPROM 5

#define MONITOR_DEVICES 6
#define DEBUG_LEDS 6
#define NUM_DEVICES 5

#define TCA_ADDRESS 0x27

class Gpio : protected Task, public DebugLED {
public:
  Gpio(Mutex* wireMutex)
    : mutex(wireMutex), TCA1(TCA9555(TCA_ADDRESS)) {
    testGPIO = false;
  };
  void setup();
  void loop(Output* output, Data* data);
  bool getRelay(int relay);
  void setRelay(int relay, bool state);
  bool getOnline(int device);
  void setOnline(int device, bool state);
  bool getCommand(int device);
  void setCommand(int device);
  void setTest(bool test) {
    testGPIO = test;
  };
  void commandRelay(int device);
  byte readRelay(int device);
  bool getDebug(int pin);
  void setDebug(int pin, bool state = true);
  void clearDebug();
private:
  Mutex* mutex;
  TCA9555 TCA1;
  bool testGPIO;
  int relayStatusPins[NUM_DEVICES] = { 0, 1, 2, 3, 4 };
  bool relayStatus[NUM_DEVICES];
  int onlineStatusPins[MONITOR_DEVICES] = { 5, 6, 7, 8, 9, 10 };
  bool onlineStatus[MONITOR_DEVICES];
  int commandRelayPins[NUM_DEVICES] = { 11, 12, 13, 14, 15 };
  bool commandStatus[NUM_DEVICES];
  int debugPins[DEBUG_LEDS] = { 6, 7, 8, 9, 10, 11 };
  bool debugStatus[DEBUG_LEDS];
  void testOutput(unsigned int pin);
};

#endif
