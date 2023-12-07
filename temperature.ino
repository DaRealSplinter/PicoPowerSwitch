#ifdef TEMPERATURE
#include "temperature.h"

#define DRIFT 0

bool Temperature::validTemperature() {
  return validTemp;
}

void Temperature::setup(Gpio* gpio, EEpromMemory* memory) {
  _gpio = gpio;
  _memory = memory;
  setRefresh(refreshRateInValid);
  dht.begin();
  readTemperature();
  debugln("Temperature Sensor Complete");
}

void Temperature::loop() {
  if (run()) {
    readTemperature();
    if (validTemp) {
      setRefresh(refreshRateValid);
    } else {
      setRefresh(refreshRateInValid);
    }
  }
}

int Temperature::readTemperature() {
  temp = (dht.readTemperature(true) + _memory->mem.mem.drift);
  if ((temp < 0) || (temp > 150)) {
    validTemp = false;
  } else {
    validTemp = true;
  }
  _gpio->setOnline(DHT_11, validTemp);
  return temp;
}

int Temperature::getTemperature() {
  readTemperature();
  return temp;
}


#else
#pragma GCC warning "No Temperature Module Included"
void Temperature::setup(Gpio* gpio, EEpromMemory* memory){};
void Temperature::loop(){};
bool Temperature::validTemperature() {
  return false;
}
int Temperature::getTemperature() {
  return 0;
}
#endif
