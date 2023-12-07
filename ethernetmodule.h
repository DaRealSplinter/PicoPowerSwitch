#ifndef __ETHERNET
#define __ETHERNET

#include "defines.h"
#include "util.h"
#include "memory.h"

class EthernetModule {
public:
  EthernetModule(EEpromMemory* mem)
    : _memory(mem) {
    etherMutex = new Mutex();
  };
  void setup(Gpio* gpio);
  void loop();
  bool linkStatus();
  IPAddress getIPAddress();
  IPAddress getSubnetMask();
  IPAddress getGateway();
  IPAddress getDNS();
private:
  EEpromMemory* _memory;
  Mutex* etherMutex;
  bool setupSPI();
  bool resetW5500();
  bool setupW5500();
  void saveIPData();
};


#endif
