#define FIXED_IP 192, 168, 1, 179

#ifdef ETHERNET

#include "ethernetmodule.h"
#include "gpio.h"
#include "memory.h"
#include "temperature.h"
#include "screen.h"

bool EthernetModule::setupSPI() {
  bool status = true;
  status &= SPI.setSCK(18);
  status &= SPI.setTX(19);
  status &= SPI.setRX(16);
  status &= SPI.setCS(17);

  SPI.begin();
  debugln("SPI Start Complete");
  return status;
}


bool EthernetModule::resetW5500() {
  pinMode(15, OUTPUT);
  digitalWrite(15, LOW);
  delay(200);
  digitalWrite(15, HIGH);
  delay(200);
  debugln("W5500 Restart Complete");
  return true;
}

bool EthernetModule::setupW5500() {
  bool status = true;
  status &= resetW5500();
  Ethernet.init(17);
  if (_memory->mem.mem.isDHCP) {
    Ethernet.begin(_memory->mem.mem.macAddress);
    saveIPData();
  } else {
    Ethernet.begin(_memory->mem.mem.macAddress, _memory->mem.mem.ipAddress, _memory->mem.mem.dnsAddress, _memory->mem.mem.gatewayAddress, _memory->mem.mem.subnetMask);
  }

  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    debugln("Ethernet Module was not found.");
  } else if (Ethernet.hardwareStatus() == EthernetW5100) {
    debugln("W5100 Ethernet controller detected.");
  } else if (Ethernet.hardwareStatus() == EthernetW5200) {
    debugln("W5200 Ethernet controller detected.");
  } else if (Ethernet.hardwareStatus() == EthernetW5500) {
    status &= true;
    debugln("W5500 Ethernet controller detected.");
  }
  return status;
}

void EthernetModule::setup(Gpio* gpio) {
  bool status = true;
  etherMutex->take();
  status &= setupSPI();
  status &= setupW5500();
  etherMutex->give();
  debugln("Ethernet Complete");
  gpio->setOnline(W5500, status);
  return;
}

void EthernetModule::loop() {
  etherMutex->take();
  if (Ethernet.maintain() == 4) {
    saveIPData();
  }
  etherMutex->give();
}

IPAddress EthernetModule::getIPAddress() {
  IPAddress address;
  etherMutex->take();
  address = Ethernet.localIP();
  etherMutex->give();
  return address;
}

IPAddress EthernetModule::getDNS() {
  IPAddress address;
  etherMutex->take();
  address = Ethernet.dnsServerIP();
  etherMutex->give();
  return address;
}

IPAddress EthernetModule::getSubnetMask() {
  IPAddress address;
  etherMutex->take();
  address = Ethernet.subnetMask();
  etherMutex->give();
  return address;
}

IPAddress EthernetModule::getGateway() {
  IPAddress address;
  etherMutex->take();
  address = Ethernet.gatewayIP();
  etherMutex->give();
  return address;
}

bool EthernetModule::linkStatus() {
  bool status;
  etherMutex->take();
  status = Ethernet.linkStatus();
  etherMutex->give();
  return status;
}
void EthernetModule::saveIPData() {
  IPAddress address;
  address = Ethernet.localIP();
  _memory->mem.mem.ipAddress[0] = address[0];
  _memory->mem.mem.ipAddress[1] = address[1];
  _memory->mem.mem.ipAddress[2] = address[2];
  _memory->mem.mem.ipAddress[3] = address[3];
  address = Ethernet.dnsServerIP();
  _memory->mem.mem.dnsAddress[0] = address[0];
  _memory->mem.mem.dnsAddress[1] = address[1];
  _memory->mem.mem.dnsAddress[2] = address[2];
  _memory->mem.mem.dnsAddress[3] = address[3];
  address = Ethernet.gatewayIP();
  _memory->mem.mem.gatewayAddress[0] = address[0];
  _memory->mem.mem.gatewayAddress[1] = address[1];
  _memory->mem.mem.gatewayAddress[2] = address[2];
  _memory->mem.mem.gatewayAddress[3] = address[3];
  address = Ethernet.subnetMask();
  _memory->mem.mem.subnetMask[0] = address[0];
  _memory->mem.mem.subnetMask[1] = address[1];
  _memory->mem.mem.subnetMask[2] = address[2];
  _memory->mem.mem.subnetMask[3] = address[3];
  _memory->breakSeal();
}


#else
#pragma GCC warning "No Ethernet Module Included"

void EthernetModule::setup(Gpio* gpio){};
void EthernetModule::loop(){};
IPAddress ethernetIP(255, 255, 255, 255);
IPAddress EthernetModule::getIPAddress() {
  return ethernetIP;
};
IPAddress EthernetModule::getSubnetMask() {
  return ethernetIP;
}
IPAddress EthernetModule::getDNS() {
  return ethernetIP;
}
IPAddress EthernetModule::getGateway() {
  return ethernetIP;
}
bool EthernetModule::linkStatus() {
  return false;
};
#endif
