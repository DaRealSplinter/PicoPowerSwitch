#ifndef __TEMPERATURE
#define __TEMPERATURE

#include <DHT.h>
#include "gpio.h"
#include "memory.h"

class Temperature : protected Task {
public:
  Temperature()
    : temp(0), validTemp(false), dht(DHT(28, DHT11)){};
  void setup(Gpio* gpio, EEpromMemory* memory);
  void loop();
  bool validTemperature();
  int getTemperature();
private:
  int temp;
  bool validTemp;
  DHT dht;
  Gpio* _gpio;
  EEpromMemory* _memory;
  int readTemperature();
  const unsigned long refreshRateValid = 60000;
  const unsigned long refreshRateInValid = 4000;
};

#endif
