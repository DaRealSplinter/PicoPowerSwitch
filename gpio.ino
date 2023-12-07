#include <Wire.h>
#include "gpio.h"

#ifdef GPIO

void Gpio::setup() {
  mutex->take();

  TCA1.begin();
  for (int i = 0; i < NUM_DEVICES; i++) {
    TCA1.pinMode(relayStatusPins[i], INPUT);
    relayStatus[i] = TCA1.digitalRead(relayStatusPins[i]);
    commandStatus[i] = false;
    TCA1.digitalWrite(commandRelayPins[i], LOW);
    TCA1.pinMode(commandRelayPins[i], OUTPUT);
  }
  for (int i = 0; i < MONITOR_DEVICES; i++) {
    onlineStatus[i] = false;
    TCA1.pinMode(onlineStatusPins[i], OUTPUT);
    TCA1.digitalWrite(onlineStatusPins[i], HIGH);
  }
  for (int i = 0; i < DEBUG_LEDS; i++) {
    debugStatus[i] = false;
    pinMode(debugPins[i], OUTPUT);
    digitalWrite(debugPins[i], HIGH);
  }
  mutex->give();
  setOnline(BUTTON, true);
  debugln("GPIO Setup Complete");
}

void Gpio::testOutput(unsigned int pin) {
  mutex->take();
  TCA1.digitalWrite(pin, HIGH);
  delay(500);
  TCA1.digitalWrite(pin, LOW);
  mutex->give();
}

void Gpio::loop(Output* output, Data* data) {
  if (run()) {
    if (testGPIO == true) {
      for (int i = 0; i < MONITOR_DEVICES; i++) testOutput(onlineStatusPins[i]);
      for (int i = 0; i < DEBUG_LEDS; i++)
        ;  //testOutput(debugPins[i]);
      for (int i = 0; i < NUM_DEVICES; i++) testOutput(commandRelayPins[i]);
      delay(1000);
    }
    for (int i = 0; i < NUM_DEVICES; i++) {
      mutex->take();
      byte status = readRelay(i);
      mutex->give();
      if (status != relayStatus[i]) {
        String caption = String(data->getDeviceName(i) + String(" ") + String((status) ? "ON" : "OFF"));
        output->setScreen(LIGHT, caption);
        //output->setScreen("Power Changed", String(i + 1), data->getDeviceName(i), "", String((status) ? "ON" : "OFF"));
      }
      relayStatus[i] = status;
#ifdef BACKUP_INDICATORS
        debugStatus[i+1] = status;
#endif
    }
    for (int i = 0; i < MONITOR_DEVICES; i++) {
      mutex->take();
      TCA1.digitalWrite(onlineStatusPins[i], !onlineStatus[i]);
      mutex->give();
    }
    for (int i = 0; i < DEBUG_LEDS; i++) {
      mutex->take();
      digitalWrite(debugPins[i], !debugStatus[i]);
      mutex->give();
    }
    for (int i = 0; i < NUM_DEVICES; i++) {
      if (commandStatus[i] == true) {
        mutex->take();
        commandRelay(i);
        commandStatus[i] = false;
        mutex->give();
      }
    }
  }
}

byte Gpio::readRelay(int device) {
  return TCA1.digitalRead(relayStatusPins[device]);
}

void Gpio::commandRelay(int device) {
  TCA1.digitalWrite(commandRelayPins[device], HIGH);
  delay(500);
  TCA1.digitalWrite(commandRelayPins[device], LOW);
}

void Gpio::setRelay(int relay, bool state) {
  if ((relay >= 0) && (relay < NUM_DEVICES))
    relayStatus[relay] = state;
}

bool Gpio::getRelay(int relay) {
  if ((relay >= 0) && (relay < NUM_DEVICES))
    return relayStatus[relay];
  return false;
}

void Gpio::setDebug(int pin, bool state) {
  if ((pin >= 0) && (pin < DEBUG_LEDS)) {
    debugStatus[pin] = state;
    mutex->take();
    digitalWrite(debugPins[pin], !debugStatus[pin]);
    mutex->give();
  }
}

bool Gpio::getDebug(int pin) {
  if ((pin >= 0) && (pin < DEBUG_LEDS))
    return debugStatus[pin];
  return false;
}

void Gpio::clearDebug() {
  for (int i = 0; i < DEBUG_LEDS; i++) {
    debugStatus[i] = false;
    mutex->take();
    digitalWrite(debugPins[i], !debugStatus[i]);
    mutex->give();
  }
}

void Gpio::setOnline(int device, bool state) {
  if ((device >= 0) && (device < MONITOR_DEVICES))
    onlineStatus[device] = state;
}

bool Gpio::getOnline(int device) {
  if ((device >= 0) && (device < MONITOR_DEVICES))
    return onlineStatus[device];
  return false;
}

void Gpio::setCommand(int device) {
  if ((device >= 0) && (device < NUM_DEVICES))
    commandStatus[device] = true;
}

bool Gpio::getCommand(int device) {
  if ((device >= 0) && (device < NUM_DEVICES))
    return commandStatus[device];
  return false;
}

#else
#pragma GCC warning "No GPIO Module Included"
void Gpio::setup(){};
void Gpio::loop(Output* output, Data* data){};
bool Gpio::getRelay(int relay) {
  return false;
};
void Gpio::setDebug(int pin, bool state){};
void Gpio::clearDebug(){};
void Gpio::setOnline(int device, bool state){};
void Gpio::setCommand(int device){};
bool Gpio::getDebug(int pin) {
  return false;
};
bool Gpio::getOnline(int device) {
  return false;
};
bool Gpio::getCommand(int device) {
  return false;
};
byte Gpio::readRelay(int device) {
  return 0;
}
void Gpio::commandRelay(int device) {
}
#endif
