#ifndef __MEMORY
#define __MEMORY

#include <I2C_eeprom.h>
#include "util.h"
#include "gpio.h"

#define PROGRAM_NUMBER 0x01
#define PROGRAM_VERSION_MAJOR 0x01
#define PROGRAM_VERSION_MINOR 0x06
#define NAME_MAX_LENGTH 15

struct MemoryStruct {
  byte programNumber;
  byte programVersionMajor;
  byte programVersionMinor;
  byte macAddress[6];
  bool isDHCP;
  byte ipAddress[4];
  byte dnsAddress[4];
  byte subnetMask[4];
  byte gatewayAddress[4];
  byte numberOfDevices;
  char deviceName[NUM_DEVICES][NAME_MAX_LENGTH + 1];
  int drift;
};

typedef union {
  MemoryStruct mem;
  byte memoryArray[sizeof(MemoryStruct)];
} Memory;


class EEpromMemory : public Data, protected Task {
public:
  EEpromMemory(Mutex* mutex)
    : wireMutex(mutex) {
    String error = "ERROR";
    error.toCharArray(ErrorString, NAME_MAX_LENGTH);
    memMutex = new Mutex();
    eeprom = new I2C_eeprom(0x50, I2C_DEVICESIZE_24LC16);
  };
  void setup(Gpio* gpio);
  void loop();
  byte getNumberOfDevices() {
    return mem.mem.numberOfDevices;
  };
  char* getDeviceName(byte device);
  void setDeviceName(byte device, char* name, int length);
  void initMemory();
  Memory mem;
  void readEEPROM();
  void writeEEPROM();
  void breakSeal();
private:
  char ErrorString[NAME_MAX_LENGTH];
  Mutex* wireMutex;
  Mutex* memMutex;
  bool seal;
  byte readEEPROMbyte(unsigned int address);
  void writeEEPROMbyte(unsigned int address, byte value);
  bool connected;
  I2C_eeprom* eeprom;
};

#endif
