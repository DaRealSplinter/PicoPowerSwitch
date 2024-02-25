#include "SerialCmd.h"

#ifndef GAVEL_SERIAL_CMD
#error "Serial Commands are not echoing, plus backspace support"
#endif
#if (SERIALCMD_MAXCMDNUM != 16)
#error "Serial Commands not set to correct value"
#endif


#include "serialport.h"
#include "util.h"

#ifdef SERIALPORT

static Mutex serialMutex;

void serialSetup() {
  Serial1.begin(115200);
}

static void __print(String line) {
  serialMutex.take();
  Serial1.print(line);
  serialMutex.give();
}
static void __println(String line) {
  serialMutex.take();
  Serial1.println(line);
  serialMutex.give();
}

void printColor(COLOR color) {
  char colorString[32];
  sprintf(colorString, "\033[%dm", color);
  __print(colorString);
}

void print(PRINT_TYPES type, String line) {
  printColor(Normal);
  switch (type) {
    case TRACE:
#ifdef DEBUG
      __print("[");
      printColor(Cyan);
      __print("  DEBUG ");
      printColor(Normal);
      __print("] ");
      printColor(Cyan);
#else
      return;
#endif
      break;
    case PROMPT:
      printColor(Green);
      break;
    case ERROR:
      __print("[");
      printColor(Red);
      __print("  ERROR ");
      printColor(Normal);
      __print("] ");
      printColor(Red);
      break;
    case PASSED:
      __print("[");
      printColor(Green);
      __print("   OK   ");
      printColor(Normal);
      __print("] ");
      break;
    case FAILED:
      __print("[");
      printColor(Red);
      __print(" FAILED ");
      printColor(Normal);
      __print("] ");
      break;
    case WARNING:
      printColor(Red);
      break;
    case HELP:
    case INFO:
    default:
      break;
  }
  __print(line);
  printColor(Normal);
}

void print(PRINT_TYPES type, String line, String line2) {
  printColor(Normal);
  switch (type) {
    case HELP:
      print(type, line);
      printColor(Yellow);
      __print(line2);
      printColor(Normal);
      break;
    case ERROR:
      print(type, line);
      print(WARNING, line2);
      break;
    case PASSED:
    case FAILED:
      print(type, line);
      print(INFO, line2);
      break;
    default:
      print(type, line);
      print(type, line2);
      break;
  }
}
void println() {
  __println(String(""));
}

void println(PRINT_TYPES type, String line) {
  print(type, line);
  println();
}

void println(PRINT_TYPES type, String line, String line2) {
  print(type, line, line2);
  println();
}

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
  serialCmd.AddCmd("?", SERIALCMD_FROMALL, help);
  serialCmd.AddCmd("help", SERIALCMD_FROMALL, help);
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
#else
#pragma GCC warning "Debug Functions are defined but unused - Expected Behaviour!"
#endif
  println(PASSED, "Serial Port Complete");
}

void SerialPort::loop() {
  int ret;
  ret = serialCmd.ReadSer();
  if (ret == 0) {
    println();
    print(ERROR, "Unrecognized command: ");
    println(WARNING, serialCmd.lastLine);
    println(INFO, "Enter \'?\' for help.");
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
  println();
  value = serialCmd.ReadNext();
  if (value != NULL) {
    BITMAP bitmap = (BITMAP)atoi(value);
    _screen->setScreen(bitmap);
  } else
    help();
}
static void minimalist() {
  println();
  println(INFO, "For use in automation, there are minimalist HTTP requests and responses.");
  println(INFO, "To make use of these, open a telnet session to the power module on port 80.");
  println(INFO, "To turn on a switch - \"GET /on/[n]\" the response is \"on\".");
  println(INFO, "To turn off a switch - \"GET /off/[n]\" the response is \"off\".");
  println(INFO, "To check the status of a switch - \"GET /stat/[n]\" the response is \"on\" or \"off\".");
  println(INFO, "[n] is the switch number between 1 and " + String(NUM_DEVICES));
  println();
  println(INFO, "The purpose of these minimalist http commands is to enable the coder");
  println(INFO, "to make use of the power switch without the overhead of http.");
  prompt();
}

void deleteFile() {
  char* value;
  println();
  value = serialCmd.ReadNext();
  if (value != NULL) {
    _files->deleteFile(value);
  } else {
    help();
  }
}

void printDir() {
  println();
  _files->printInfo();
}

void testOutput() {
  int numberOfDevices = _memory->getNumberOfDevices();
  bool firstState;
  bool secondState;
  bool thirdState;
  bool error = false;
  bool overall = false;
  println();
  println(INFO, "Testing Output Ports");
  print(INFO, "Output Ports: ");
  println(INFO, String(numberOfDevices));
  for (int i = 0; i < numberOfDevices; i++) {
    error = false;
    rp2040.wdt_reset();
    print(INFO, "Testing Port: ");
    print(INFO, String(i + 1));
    print(INFO, "; Name: ");
    println(INFO, String(_memory->getDeviceName(i)));
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
      print(ERROR, "Output Port ");
      print(WARNING, String(i + 1));
      println(WARNING, " Failed");
      print(ERROR, "First State: ");
      println(WARNING, (firstState) ? "HIGH" : "LOW");
      print(ERROR, "Second State: ");
      println(WARNING, (secondState) ? "HIGH" : "LOW");
      print(ERROR, "Third State: ");
      println(WARNING, (thirdState) ? "HIGH" : "LOW");
    } else {
      print(INFO, "SUCCESS: Output Port ");
      print(INFO, String(i + 1));
      println(INFO, " Passed");
    }
    overall |= error;
  }
  if (overall) println(FAILED, "ERROR: Output Port Test Failed");
  else println(PASSED, "SUCCESS: Output Port Test Passed");
}

void memStat() {
  println();
  print(INFO, "PRG Num: ");
  print(INFO, String(_memory->mem.mem.programNumber));
  print(INFO, " PRG Ver: ");
  print(INFO, String(_memory->mem.mem.programVersionMajor));
  print(INFO, ".");
  println(INFO, String(_memory->mem.mem.programVersionMinor));
  print(INFO, "MAC: ");
  print(INFO, String(_memory->mem.mem.macAddress[0], HEX) + ":");
  print(INFO, String(_memory->mem.mem.macAddress[1], HEX) + ":");
  print(INFO, String(_memory->mem.mem.macAddress[2], HEX) + ":");
  print(INFO, String(_memory->mem.mem.macAddress[3], HEX) + ":");
  print(INFO, String(_memory->mem.mem.macAddress[4], HEX) + ":");
  println(INFO, String(_memory->mem.mem.macAddress[5], HEX));
  println(INFO, "IP Address is " + String((_memory->mem.mem.isDHCP) ? "DHCP" : "Static"));
  print(INFO, "IP Address: ");
  print(INFO, String(_memory->mem.mem.ipAddress[0]) + ".");
  print(INFO, String(_memory->mem.mem.ipAddress[1]) + ".");
  print(INFO, String(_memory->mem.mem.ipAddress[2]) + ".");
  println(INFO, String(_memory->mem.mem.ipAddress[3]));
  print(INFO, "Subnet Mask: ");
  print(INFO, String(_memory->mem.mem.subnetMask[0]) + ".");
  print(INFO, String(_memory->mem.mem.subnetMask[1]) + ".");
  print(INFO, String(_memory->mem.mem.subnetMask[2]) + ".");
  println(INFO, String(_memory->mem.mem.subnetMask[3]));
  print(INFO, "Gateway: ");
  print(INFO, String(_memory->mem.mem.gatewayAddress[0]) + ".");
  print(INFO, String(_memory->mem.mem.gatewayAddress[1]) + ".");
  print(INFO, String(_memory->mem.mem.gatewayAddress[2]) + ".");
  println(INFO, String(_memory->mem.mem.gatewayAddress[3]));
  print(INFO, "DNS Address: ");
  print(INFO, String(_memory->mem.mem.dnsAddress[0]) + ".");
  print(INFO, String(_memory->mem.mem.dnsAddress[1]) + ".");
  print(INFO, String(_memory->mem.mem.dnsAddress[2]) + ".");
  println(INFO, String(_memory->mem.mem.dnsAddress[3]));
  print(INFO, "Num Dev: ");
  println(INFO, String(_memory->mem.mem.numberOfDevices));
  for (int i = 0; i < _memory->getNumberOfDevices(); i++) {
    print(INFO, "Name ");
    print(INFO, String(i + 1));
    print(INFO, ": ");
    println(INFO, String(_memory->getDeviceName(i)));
  }
  print(INFO, "Temperature Drift: ");
  println(INFO, String(_memory->mem.mem.drift));
}

void ipStat() {
  IPAddress ipAddress = ethernet.getIPAddress();
  bool linked = ethernet.linkStatus();
  println();
  print(INFO, "MAC Address:  ");
  print(INFO, String(_memory->mem.mem.macAddress[0], HEX) + ":");
  print(INFO, String(_memory->mem.mem.macAddress[1], HEX) + ":");
  print(INFO, String(_memory->mem.mem.macAddress[2], HEX) + ":");
  print(INFO, String(_memory->mem.mem.macAddress[3], HEX) + ":");
  print(INFO, String(_memory->mem.mem.macAddress[4], HEX) + ":");
  println(INFO, String(_memory->mem.mem.macAddress[5], HEX));
  println(INFO, "IP Address is " + String((_memory->mem.mem.isDHCP) ? "DHCP" : "Static"));
  println(INFO, String((linked) ? "Connected" : "Unconnected"));
  println(INFO, "  IP Address:  " + String(ipAddress[0]) + String(".") + String(ipAddress[1]) + String(".") + String(ipAddress[2]) + String(".") + String(ipAddress[3]));
  ipAddress = ethernet.getSubnetMask();
  println(INFO, "  Subnet Mask: " + String(ipAddress[0]) + String(".") + String(ipAddress[1]) + String(".") + String(ipAddress[2]) + String(".") + String(ipAddress[3]));
  ipAddress = ethernet.getGateway();
  println(INFO, "  Gateway:     " + String(ipAddress[0]) + String(".") + String(ipAddress[1]) + String(".") + String(ipAddress[2]) + String(".") + String(ipAddress[3]));
  ipAddress = ethernet.getDNS();
  println(INFO, "  DNS Server:  " + String(ipAddress[0]) + String(".") + String(ipAddress[1]) + String(".") + String(ipAddress[2]) + String(".") + String(ipAddress[3]));
}

void rebootPico() {
  println();
  println(PROMPT, "Rebooting....");
  _screen->setScreen("Rebooting...");
  delay(100);
  rp2040.reboot();
}

void wipeMemory() {
  println();
  _memory->initMemory();
}

void relayToggle() {
  char* value;
  println();
  value = serialCmd.ReadNext();
  if (value != NULL) {
    int relay = atoi(value) - 1;
    _gpio->setCommand(relay);
    print(INFO, "Commanded Relay ");
    print(INFO, value);
    println(INFO, String(!_gpio->getRelay(relay) ? " ON" : " OFF"));
    _screen->setScreen("Commanded Relay", "", "Relay " + String(value), (!_gpio->getRelay(relay)) ? "ON" : "OFF");
  } else
    help();
}

void stat() {
  char* value;
  println();
  value = serialCmd.ReadNext();
  if (value != NULL) {
    int relay = atoi(value) - 1;
    _gpio->getRelay(relay);
    println(INFO, (_gpio->getRelay(relay)) ? "ON" : "OFF");
    _screen->setScreen("Status Relay", "", "Relay " + String(value), (_gpio->getRelay(relay)) ? "ON" : "OFF");
  } else
    help();
}

void tempstatus() {
  println();
  print(INFO, "Temperature ");
  (_temperature->validTemperature()) ? print(INFO, "valid ") : print(INFO, "invalid ");
  print(INFO, String(_temperature->getTemperature()));
  println(INFO, "°F.");
  _screen->setScreen("Temperature", "", (_temperature->validTemperature()) ? "Valid " : "Invalid ", String(_temperature->getTemperature()) + " F.");
}

void gpiostatus() {
  int index;
  println();
  println(INFO, "GPIO Status");
  println();
  for (index = 0; index < MONITOR_DEVICES; index++) {
    print(INFO, "Device ");
    print(INFO, String(index + 1));
    print(INFO, ": ");
    println(INFO, String(_gpio->getOnline(index)));
  }
  println();
  for (index = 0; index < NUM_DEVICES; index++) {
    print(INFO, "Relay ");
    print(INFO, String(index + 1));
    print(INFO, ": ");
    println(INFO, String(_gpio->getRelay(index)));
  }
  println();
  for (index = 0; index < NUM_DEVICES; index++) {
    print(INFO, "Command ");
    print(INFO, String(index + 1));
    print(INFO, ": ");
    println(INFO, String(_gpio->getCommand(index)));
  }
  println();
}

void scani2c() {
  println();
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

  println();
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
  } else if ((strncmp("?", value, 1) == 0) || (strncmp("help", value, 1))) {
    item = None;
    count = 0;
  } else {
    print(WARNING, "Invalid Config: <");
    print(WARNING, value);
    println(WARNING, ">");
    item = None;
  }
  for (unsigned int i = 0; i < count; i++) {
    value = serialCmd.ReadNext();
    print(INFO, value);
    print(INFO, " ");
    if (value == NULL) {
      item = None;
      count = 0;
      println(WARNING, "Missing Parameters in config");
      break;
    }
    parameters[i] = atoi(value);
  }
  value = serialCmd.ReadNext();
  println(INFO, value);
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
        println(WARNING, "Missing device name in \"config name [n] [name]\"");
        break;
      }
      _memory->setDeviceName(parameters[0] - 1, value, strlen(value));
      break;
    case None:
    default:
      println(HELP, "config name [n] [name] ", "- Sets device name");
      println(HELP, "config temp [n]        ", "- Set the drift for the temperature sensor");
      println(HELP, "config dhcp [0|1]      ", "- 0, turns off DHCP; 1, turns on DHCP");
      println(HELP, "config ip [n] [n] [n] [n]     ", "- Sets the IP address n.n.n.n");
      println(HELP, "config dns [n] [n] [n] [n]    ", "- Sets the DNS address n.n.n.n");
      println(HELP, "config gw [n] [n] [n] [n]     ", "- Sets the Gateway address n.n.n.n");
      println(HELP, "config subnet [n] [n] [n] [n] ", "- Sets the Subnet Mask n.n.n.n");
#ifdef DEBUG
      println(HELP, "config gpio [0|1]      ", "- 0, turns off GPIO testing; 1, turns on GPIO testing");
#endif
      println(HELP, "config help/?          ", "- Print config Help");
      println();
      println(HELP, "Note: Addresses use a space seperator, so \"192.168.168.4\" is \"192 168 168 4\"");
      println(HELP, "      Must Reboot the system for some changes to take effect");
      return;
  }
  println(PASSED, "Command Complete");
}

void banner() {
  println();
  println(PROMPT, APP_NAME);
  print(INFO, "PRG Num: ");
  print(INFO, String(_memory->mem.mem.programNumber));
  print(INFO, " PRG Ver: ");
  print(INFO, String(_memory->mem.mem.programVersionMajor));
  print(INFO, ".");
  print(INFO, String(_memory->mem.mem.programVersionMinor));
  print(INFO, " Num Dev: ");
  println(INFO, String(_memory->mem.mem.numberOfDevices));
  println(INFO, "Build Date: " + String(compileDate) + " Time: " + String(compileTime));
  println();
  print(INFO, "Microcontroller: Raspberry Pi Pico");
  (rp2040.isPicoW()) ? println(INFO, "W") : println();
  print(INFO, "Core is running at ");
  print(INFO, String(rp2040.f_cpu() / 1000000));
  println(INFO, " Mhz");
  int used = rp2040.getUsedHeap();
  int total = rp2040.getTotalHeap();
  int percentage = (used * 100) / total;
  print(INFO, "RAM Memory Usage: ");
  print(INFO, String(used));
  print(INFO, "/");
  print(INFO, String(total));
  print(INFO, " --> ");
  print(INFO, String(percentage));
  println(INFO, "%");
  print(INFO, "CPU Temperature: ");
  print(INFO, String((9.0 / 5.0 * analogReadTemp()) + 32.0, 0));
  println(INFO, "°F.");
}

void help() {
  banner();
  println();

  println(HELP, "config ... ", "- Configure Devices \"config ?\" for more");
  println(HELP, "wipe       ", "- Wipe and Initialize Memory");
  println(HELP, "relay [n]  ", "- Command Relay n to Toggle");
  println(HELP, "stat [n]   ", "- Status Relay n");
  println(HELP, "temp       ", "- Temperature Status");
  println(HELP, "gpio       ", "- GPIO Status");
#ifdef DEBUG
  println(HELP, "scan       ", "- I2c Scanner");
  println(HELP, "test       ", "- Tests all of the Relay Commands and Status GPIO");
  println(HELP, "bitmap [n] ", "- Displays an image on the screen");
#endif
  println(HELP, "ip         ", "- IP Stats");
  println(HELP, "mem        ", "- Contents of Flash Memory");
  println(HELP, "dir        ", "- Contents of File System");
  println(HELP, "del [file] ", "- Deletes a file");
  println(HELP, "reboot     ", "- Software Reboot the Pico");
  println(HELP, "help/?     ", "- Print Help");
}

void prompt() {
  print(PROMPT, "power:\\> ");
}

#else
#pragma GCC warning "No Serial Port Command Server Included"
void serialSetup() {}
void print(String line) {}
void println(String line) {}
void SerialPort::setup(Scan* scan, EEpromMemory* eepromMemory, Gpio* gpioControl, Screen* oledscreen, Temperature* tempStatus, Watchdog* watchdog, Files* files){};
void SerialPort::loop(){};
void SerialPort::complete(){};
void SerialPort::banner(){};
#endif
