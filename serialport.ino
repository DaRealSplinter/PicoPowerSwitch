#include "SerialCmd.h"

#ifndef GAVEL_SERIAL_CMD
#error "Serial Commands are not echoing, plus backspace support"
#endif
#if (SERIALCMD_MAXCMDNUM != 16)
#error "Serial Commands not set to correct value"
#endif


#include "serialport.h"
#include "util.h"

static Mutex serialMutex;
static bool serialSetup = false;

#if defined(DEBUG) || defined(SERIALPORT)
void debugSetup() {
  if (!serialSetup)
    Serial1.begin(115200);
  serialSetup = true;
}

void debug(String line) {
  serialMutex.take();
  Serial1.print(line);
  serialMutex.give();
}
void debugln(String line) {
  serialMutex.take();
  Serial1.println(line);
  serialMutex.give();
}
void debugln() {
  debugln("");
}

#else
#pragma GCC warning "No Debug Output Included"
void debugSetup() {}
void debug(String line) {}
void debugln(String line) {}
#endif

#ifdef SERIALPORT
SerialCmd serialCmd(Serial1, SERIALCMD_CR, (char*)(SERIALCMD_SPACE));

static Scan* _i2cScanner;
static EEpromMemory* _memory;
static Gpio* _gpio;
static Screen* _screen;
static Temperature* _temperature;
static Watchdog* _watchdog;
static Files* _files;

static void deleteFile();
static void minimalist();
static void bitmap();
static void printDir();
static void testOutput();
static void stat();
static void configure();
static void memStat();
static void ipStat();
static void rebootPico();
static void wipeMemory();
static void relayToggle();
static void tempstatus();
static void gpiostatus();
static void scani2c();
static void help();
static void banner();
static void prompt();

void SerialPort::setup(Scan* scan, EEpromMemory* eepromMemory, Gpio* gpioControl, Screen* oledscreen, Temperature* tempStatus, Watchdog* watchdog, Files* files) {
  _i2cScanner = scan;
  _memory = eepromMemory;
  _gpio = gpioControl;
  _screen = oledscreen;
  _temperature = tempStatus;
  _watchdog = watchdog;
  _files = files;
  if (!serialSetup)
    Serial1.begin(115200);
  serialSetup = true;
  serialCmd.AddCmd("?", SERIALCMD_FROMALL, help);
  serialCmd.AddCmd("gpio", SERIALCMD_FROMALL, gpiostatus);
  serialCmd.AddCmd("temp", SERIALCMD_FROMALL, tempstatus);
  serialCmd.AddCmd("relay", SERIALCMD_FROMALL, relayToggle);
  serialCmd.AddCmd("stat", SERIALCMD_FROMALL, stat);
  serialCmd.AddCmd("wipe", SERIALCMD_FROMALL, wipeMemory);
  serialCmd.AddCmd("reboot", SERIALCMD_FROMALL, rebootPico);
  serialCmd.AddCmd("ip", SERIALCMD_FROMALL, ipStat);
  serialCmd.AddCmd("mem", SERIALCMD_FROMALL, memStat);
  serialCmd.AddCmd("config", SERIALCMD_FROMALL, configure);
  serialCmd.AddCmd("dir", SERIALCMD_FROMALL, printDir);
  serialCmd.AddCmd("min", SERIALCMD_FROMALL, minimalist);
  serialCmd.AddCmd("del", SERIALCMD_FROMALL, deleteFile);
#ifdef DEBUG
  serialCmd.AddCmd("test", SERIALCMD_FROMALL, testOutput);
  serialCmd.AddCmd("scan", SERIALCMD_FROMALL, scani2c);
  serialCmd.AddCmd("bitmap", SERIALCMD_FROMALL, bitmap);
#endif
  debugln("Serial Port Complete");
}

void SerialPort::loop() {
  int ret;
  ret = serialCmd.ReadSer();
  if (ret == 0) {
    debugln();
    debug("ERROR: Unrecognized command: ");
    debugln(serialCmd.lastLine);
    debugln("Enter \'?\' for help.");
    prompt();
  } else if (ret == 1) {
    prompt();
  }
}

void SerialPort::complete() {
  banner();
  prompt();
}

void bitmap() {
  char* value;
  debugln();
  value = serialCmd.ReadNext();
  if (value != NULL) {
    BITMAP bitmap = (BITMAP)atoi(value);
    _screen->setScreen(bitmap);
  } else
    help();
}
static void minimalist() {
  debugln();
  debugln("For use in automation, there are minimalist HTTP requests and responses.");
  debugln("To make use of these, open a telnet session to the power module on port 80.");
  debugln("To turn on a switch - \"GET /on/[n]\" the response is \"on\".");
  debugln("To turn off a switch - \"GET /off/[n]\" the response is \"off\".");
  debugln("To check the status of a switch - \"GET /stat/[n]\" the response is \"on\" or \"off\".");
  debugln("[n] is the switch number between 1 and " + String(NUM_DEVICES));
  debugln();
  debugln("The purpose of these minimalist http commands is to enable the coder");
  debugln("to make use of the power switch without the overhead of http.");
  prompt();
}

void deleteFile() {
  char* value;
  debugln();
  value = serialCmd.ReadNext();
  if (value != NULL) {
    _files->deleteFile(value);
  } else {
    help();
  }
}

void printDir() {
  debugln();
  _files->printInfo();
}

void testOutput() {
  int numberOfDevices = _memory->getNumberOfDevices();
  bool firstState;
  bool secondState;
  bool thirdState;
  bool error = false;
  bool overall = false;
  debugln();
  debugln("Testing Output Ports");
  debug("Output Ports: ");
  debugln(String(numberOfDevices));
  for (int i = 0; i < numberOfDevices; i++) {
    error = false;
    rp2040.wdt_reset();
    debug("Testing Port: ");
    debug(String(i + 1));
    debug("; Name: ");
    debugln(String(_memory->getDeviceName(i)));
    firstState = _gpio->readRelay(i);
    _gpio->commandRelay(i);
    delay(1000);
    secondState = _gpio->readRelay(i);
    _gpio->commandRelay(i);
    delay(1000);
    thirdState = _gpio->readRelay(i);

    if (firstState == secondState)
      error = true;
    if (thirdState == secondState)
      error = true;
    if (thirdState != firstState)
      error = true;

    if (error) {
      debug("ERROR: Output Port ");
      debug(String(i + 1));
      debugln(" Failed");
      debug("First State: ");
      debugln((firstState) ? "HIGH" : "LOW");
      debug("Second State: ");
      debugln((secondState) ? "HIGH" : "LOW");
      debug("Third State: ");
      debugln((thirdState) ? "HIGH" : "LOW");
    } else {
      debug("SUCCESS: Output Port ");
      debug(String(i + 1));
      debugln(" Passed");
    }
    overall |= error;
  }
  if (overall) debugln("ERROR: Output Port Test Failed");
  else debugln("SUCCESS: Output Port Test Passed");
}

void memStat() {
  debugln();
  debug("PRG Num: ");
  debug(String(_memory->mem.mem.programNumber));
  debug(" PRG Ver: ");
  debug(String(_memory->mem.mem.programVersionMajor));
  debug(".");
  debugln(String(_memory->mem.mem.programVersionMinor));
  debug("MAC: ");
  debug(String(_memory->mem.mem.macAddress[0], HEX) + ":");
  debug(String(_memory->mem.mem.macAddress[1], HEX) + ":");
  debug(String(_memory->mem.mem.macAddress[2], HEX) + ":");
  debug(String(_memory->mem.mem.macAddress[3], HEX) + ":");
  debug(String(_memory->mem.mem.macAddress[4], HEX) + ":");
  debugln(String(_memory->mem.mem.macAddress[5], HEX));
  debugln("IP Address is " + String((_memory->mem.mem.isDHCP) ? "DHCP" : "Static"));
  debug("IP Address: ");
  debug(String(_memory->mem.mem.ipAddress[0]) + ".");
  debug(String(_memory->mem.mem.ipAddress[1]) + ".");
  debug(String(_memory->mem.mem.ipAddress[2]) + ".");
  debugln(String(_memory->mem.mem.ipAddress[3]));
  debug("Subnet Mask: ");
  debug(String(_memory->mem.mem.subnetMask[0]) + ".");
  debug(String(_memory->mem.mem.subnetMask[1]) + ".");
  debug(String(_memory->mem.mem.subnetMask[2]) + ".");
  debugln(String(_memory->mem.mem.subnetMask[3]));
  debug("Gateway: ");
  debug(String(_memory->mem.mem.gatewayAddress[0]) + ".");
  debug(String(_memory->mem.mem.gatewayAddress[1]) + ".");
  debug(String(_memory->mem.mem.gatewayAddress[2]) + ".");
  debugln(String(_memory->mem.mem.gatewayAddress[3]));
  debug("DNS Address: ");
  debug(String(_memory->mem.mem.dnsAddress[0]) + ".");
  debug(String(_memory->mem.mem.dnsAddress[1]) + ".");
  debug(String(_memory->mem.mem.dnsAddress[2]) + ".");
  debugln(String(_memory->mem.mem.dnsAddress[3]));
  debug("Num Dev: ");
  debugln(String(_memory->mem.mem.numberOfDevices));
  for (int i = 0; i < _memory->getNumberOfDevices(); i++) {
    debug("Name ");
    debug(String(i + 1));
    debug(": ");
    debugln(String(_memory->getDeviceName(i)));
  }
  debug("Temperature Drift: ");
  debugln(String(_memory->mem.mem.drift));
}

void ipStat() {
  IPAddress ipAddress = ethernet.getIPAddress();
  bool linked = ethernet.linkStatus();
  debugln();
  debug("MAC Address:  ");
  debug(String(_memory->mem.mem.macAddress[0], HEX) + ":");
  debug(String(_memory->mem.mem.macAddress[1], HEX) + ":");
  debug(String(_memory->mem.mem.macAddress[2], HEX) + ":");
  debug(String(_memory->mem.mem.macAddress[3], HEX) + ":");
  debug(String(_memory->mem.mem.macAddress[4], HEX) + ":");
  debugln(String(_memory->mem.mem.macAddress[5], HEX));
  debugln("IP Address is " + String((_memory->mem.mem.isDHCP) ? "DHCP" : "Static"));
  debugln(String((linked) ? "Connected" : "Unconnected"));
  debugln("  IP Address:  " + String(ipAddress[0]) + String(".") + String(ipAddress[1]) + String(".") + String(ipAddress[2]) + String(".") + String(ipAddress[3]));
  ipAddress = ethernet.getSubnetMask();
  debugln("  Subnet Mask: " + String(ipAddress[0]) + String(".") + String(ipAddress[1]) + String(".") + String(ipAddress[2]) + String(".") + String(ipAddress[3]));
  ipAddress = ethernet.getGateway();
  debugln("  Gateway:     " + String(ipAddress[0]) + String(".") + String(ipAddress[1]) + String(".") + String(ipAddress[2]) + String(".") + String(ipAddress[3]));
  ipAddress = ethernet.getDNS();
  debugln("  DNS Server:  " + String(ipAddress[0]) + String(".") + String(ipAddress[1]) + String(".") + String(ipAddress[2]) + String(".") + String(ipAddress[3]));
}

void rebootPico() {
  debugln();
  debugln("Rebooting....");
  _screen->setScreen("Rebooting...");
  delay(100);
  rp2040.reboot();
}

void wipeMemory() {
  debugln();
  _memory->initMemory();
}

void relayToggle() {
  char* value;
  debugln();
  value = serialCmd.ReadNext();
  if (value != NULL) {
    int relay = atoi(value) - 1;
    _gpio->setCommand(relay);
    debug("Commanded Relay ");
    debug(value);
    debugln(String(!_gpio->getRelay(relay) ? " ON" : " OFF"));
    _screen->setScreen("Commanded Relay", "", "Relay " + String(value), (!_gpio->getRelay(relay)) ? "ON" : "OFF");
  } else
    help();
}

void stat() {
  char* value;
  debugln();
  value = serialCmd.ReadNext();
  if (value != NULL) {
    int relay = atoi(value) - 1;
    _gpio->getRelay(relay);
    debugln((_gpio->getRelay(relay)) ? "ON" : "OFF");
    _screen->setScreen("Status Relay", "", "Relay " + String(value), (_gpio->getRelay(relay)) ? "ON" : "OFF");
  } else
    help();
}

void tempstatus() {
  debugln();
  debug("Temperature ");
  (_temperature->validTemperature()) ? debug("valid ") : debug("invalid ");
  debug(String(_temperature->getTemperature()));
  debugln("°F.");
  _screen->setScreen("Temperature", "", (_temperature->validTemperature()) ? "Valid " : "Invalid ", String(_temperature->getTemperature()) + " F.");
}

void gpiostatus() {
  int index;
  debugln();
  debugln("GPIO Status");
  debugln();
  for (index = 0; index < MONITOR_DEVICES; index++) {
    debug("Device ");
    debug(String(index + 1));
    debug(": ");
    debugln(String(_gpio->getOnline(index)));
  }
  debugln();
  for (index = 0; index < NUM_DEVICES; index++) {
    debug("Relay ");
    debug(String(index + 1));
    debug(": ");
    debugln(String(_gpio->getRelay(index)));
  }
  debugln();
  for (index = 0; index < NUM_DEVICES; index++) {
    debug("Command ");
    debug(String(index + 1));
    debug(": ");
    debugln(String(_gpio->getCommand(index)));
  }
  debugln();
}

void scani2c() {
  debugln();
  _i2cScanner->scan(_screen);
}

enum ConfigItem {
  None = 0,
  TempDrift,
  IpDHCP,
  IpAddress,
  IpDNS,
  IpSubnet,
  IpGW,
  GpioTest,
  Name
};

void configure() {
  char* value;
  ConfigItem item = None;
  unsigned int parameters[4];
  unsigned int count = 0;

  debugln();
  value = serialCmd.ReadNext();

  if (strncmp("temp", value, 4) == 0) {
    item = TempDrift;
    count = 1;
  } else if (strncmp("dhcp", value, 4) == 0) {
    item = IpDHCP;
    count = 1;
  } else if (strncmp("ip", value, 2) == 0) {
    item = IpAddress;
    count = 4;
  } else if (strncmp("dns", value, 3) == 0) {
    item = IpDNS;
    count = 4;
  } else if (strncmp("gw", value, 2) == 0) {
    item = IpGW;
    count = 4;
  } else if (strncmp("subnet", value, 6) == 0) {
    item = IpSubnet;
    count = 4;
#ifdef DEBUG
  } else if (strncmp("gpio", value, 4) == 0) {
    item = GpioTest;
    count = 1;
#endif
  } else if (strncmp("name", value, 4) == 0) {
    item = Name;
    count = 1;
  } else if (strncmp("?", value, 1) == 0) {
    item = None;
    count = 0;
  } else {
    debug("Invalid Config: <");
    debug(value);
    debugln(">");
    item = None;
  }
  for (unsigned int i = 0; i < count; i++) {
    value = serialCmd.ReadNext();
    debug(value);
    debug(" ");
    if (value == NULL) {
      item = None;
      count = 0;
      debugln("Missing Parameters in config");
      break;
    }
    parameters[i] = atoi(value);
  }
  value = serialCmd.ReadNext();
  debugln(value);
  switch (item) {
    case TempDrift:
      _memory->mem.mem.drift = parameters[0];
      _memory->breakSeal();
      break;
    case IpDHCP:
      _memory->mem.mem.isDHCP = parameters[0];
      _memory->breakSeal();
      break;
    case IpAddress:
      _memory->mem.mem.ipAddress[0] = parameters[0];
      _memory->mem.mem.ipAddress[1] = parameters[1];
      _memory->mem.mem.ipAddress[2] = parameters[2];
      _memory->mem.mem.ipAddress[3] = parameters[3];
      _memory->breakSeal();
      break;
    case IpDNS:
      _memory->mem.mem.dnsAddress[0] = parameters[0];
      _memory->mem.mem.dnsAddress[1] = parameters[1];
      _memory->mem.mem.dnsAddress[2] = parameters[2];
      _memory->mem.mem.dnsAddress[3] = parameters[3];
      _memory->breakSeal();
      break;
    case IpSubnet:
      _memory->mem.mem.subnetMask[0] = parameters[0];
      _memory->mem.mem.subnetMask[1] = parameters[1];
      _memory->mem.mem.subnetMask[2] = parameters[2];
      _memory->mem.mem.subnetMask[3] = parameters[3];
      _memory->breakSeal();
      break;
    case IpGW:
      _memory->mem.mem.gatewayAddress[0] = parameters[0];
      _memory->mem.mem.gatewayAddress[1] = parameters[1];
      _memory->mem.mem.gatewayAddress[2] = parameters[2];
      _memory->mem.mem.gatewayAddress[3] = parameters[3];
      _memory->breakSeal();
      break;
    case GpioTest:
      _gpio->setTest(parameters[0]);
      break;
    case Name:
      if (value == NULL) {
        debugln("Missing device name in \"config name [n] [name]\"");
        break;
      }
      _memory->setDeviceName(parameters[0] - 1, value, strlen(value));
      break;
    case None:
    default:
      debugln("config name [n] [name] - Sets device name");
      debugln("config temp [n]        - Set the drift for the temperature sensor");
      debugln("config dhcp [0|1]      - 0, turns off DHCP; 1, turns on DHCP");
      debugln("config ip [n] [n] [n] [n]     - Sets the IP address n.n.n.n");
      debugln("config dns [n] [n] [n] [n]    - Sets the DNS address n.n.n.n");
      debugln("config gw [n] [n] [n] [n]     - Sets the Gateway address n.n.n.n");
      debugln("config subnet [n] [n] [n] [n] - Sets the Subnet Mask n.n.n.n");
#ifdef DEBUG
      debugln("config gpio [0|1]      - 0, turns off GPIO testing; 1, turns on GPIO testing");
#endif
      debugln("config ?               - Print config Help");
      debugln();
      debugln("Note: Addresses use a space seperator, so \"192.168.168.4\" is \"192 168 168 4\"");
      debugln("      Must Reboot the system for some changes to take effect");
      return;
  }
  debugln("Command Complete");
}

void banner() {
  debugln();
  debugln(APP_NAME);
  debug("PRG Num: ");
  debug(String(_memory->mem.mem.programNumber));
  debug(" PRG Ver: ");
  debug(String(_memory->mem.mem.programVersionMajor));
  debug(".");
  debug(String(_memory->mem.mem.programVersionMinor));
  debug(" Num Dev: ");
  debugln(String(_memory->mem.mem.numberOfDevices));
  debugln();
  debug("Microcontroller: Raspberry Pi Pico");
  (rp2040.isPicoW()) ? debugln("W") : debugln();
  debug("Core is running at ");
  debug(String(rp2040.f_cpu() / 1000000));
  debugln(" Mhz");
  int used = rp2040.getUsedHeap();
  int total = rp2040.getTotalHeap();
  int percentage = (used * 100) / total;
  debug("RAM Memory Usage: ");
  debug(String(used));
  debug("/");
  debug(String(total));
  debug(" --> ");
  debug(String(percentage));
  debugln("%");
  debug("CPU Temperature: ");
  debug(String((9.0 / 5.0 * analogReadTemp()) + 32.0, 0));
  debugln("°F.");
}

void help() {
  banner();
  debugln();

  debugln("config ... - Configure Devices \"config ?\" for more");
  debugln("wipe       - Wipe and Initialize Memory");
  debugln("relay [n]  - Command Relay n to Toggle");
  debugln("stat [n]   - Status Relay n");
  debugln("temp       - Temperature Status");
  debugln("gpio       - GPIO Status");
#ifdef DEBUG
  debugln("scan       - I2c Scanner");
  debugln("test       - Tests all of the Relay Commands and Status GPIO");
  debugln("bitmap     - Displays an image on the screen");
#endif
  debugln("ip         - IP Stats");
  debugln("mem        - Contents of Flash Memory");
  debugln("dir        - Contents of File System");
  debugln("del [file] - Deletes a file");
  debugln("reboot     - Software Reboot the Pico");
  debugln("?          - Print Help");
}

void prompt() {
  debug("power:\\> ");
}

#else
#pragma GCC warning "No Serial Port Command Server Included"
void SerialPort::setup(Scan* scan, EEpromMemory* eepromMemory, Gpio* gpioControl, Screen* oledscreen, Temperature* tempStatus, Watchdog* watchdog){};
void SerialPort::loop(){};
#endif
