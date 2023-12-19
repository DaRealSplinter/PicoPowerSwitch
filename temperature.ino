#ifdef TEMPERATURE
#include "temperature.h"

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
  int value;
  value = ((int)dht.readTemperature(true) + _memory->mem.mem.drift);
  if ((value < 0) || (value > 150)) {
    //validTemp = false;
  } else {
    validTemp = true;
    temp = value;
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
