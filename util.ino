#include <PicoOTA.h>
#include <Wire.h>
#include "util.h"
#include "serialport.h"

void UPGRADE_SYSTEM() {
  debugln();
  debugln("Starting Upgrade Process.....");
  picoOTA.begin();
  picoOTA.addFile(UPGRADE_FILE_NAME);
  picoOTA.commit();
  LittleFS.end();
  debugln("Reboot in progress.....");
}

bool Task::run() {
  bool runTask = false;
  if (refresh > millis())
    reset();

  if ((millis() - refresh) > timeout) {
    reset();
    runTask = true;
  }
  return runTask;
}

void Task::reset() {
  refresh = millis();
}

Mutex::Mutex() {
  mutex = xSemaphoreCreateMutex();
}

void Mutex::take() {
  xSemaphoreTake(mutex, portMAX_DELAY);
}

void Mutex::give() {
  xSemaphoreGive(mutex);
}

void Watchdog::setup() {
  mutex.take();
  rp2040.wdt_begin(watchdogTimeout);
  setRefresh(watchdogPetCycle);
  petWatchdog();
  mutex.give();
}

void Watchdog::loop() {
  static int lastCore = 0;
  int core = rp2040.cpuid();
  mutex.take();
  if ((core != lastCore) && run()) {
    lastCore = core;
    petWatchdog();
  }
  mutex.give();
}

void Watchdog::petWatchdog() {
  if (resetFlag)
    rp2040.wdt_reset();
}

void Watchdog::reboot() {
  petWatchdog();
  resetFlag = false;
}

#ifdef BLINK
void Blink::loop() {
  if (run()) {
    digitalWrite(LED_BUILTIN, state);
    state = !state;
  }
}
#else
void Blink::loop(){};
#endif

#ifdef CYLON
void Cylon::loop() {
  if (run()) {
    _dbl->setDebug(abs(state), false);
    state++;
    if (state >= 5) state = -5;
    _dbl->setDebug(abs(state), true);
  }
}
#else
#ifndef BACKUP_INDICATORS
#pragma GCC warning "Debug LEDS used for something else....."
#endif
void Cylon::loop(){};
#endif

void Files::setup() {
  if (LittleFS.begin())
    debugln("File System Complete");
  else debugln("File System FAILED");
  deleteFile(UPGRADE_COMMAND_FILE_NAME);
  deleteFile(UPGRADE_FILE_NAME);
}

File Files::getFile(String path) {
  File file;
  if (LittleFS.exists(path)) {
    file = LittleFS.open(path, "r");
  } else debugln("FS ERROR: " + path + " file does not exist!!!!");

  return file;
}

void Files::deleteFile(String path) {
  File file;
  if (LittleFS.exists(path)) {
    LittleFS.remove(path);
    debugln(path + String(" file deleted."));
  } else debugln("FS ERROR: " + path + " file does not exist!!!!");
}

File Files::writeFile(String path) {
  File file;
  file = LittleFS.open(path, "w");
  if (!file) debugln("FS ERROR: " + path + " opening file for writing!!!!");

  return file;
}


void Files::printInfo() {
  int size = 0;
  int count = 0;
  FSInfo info;
  LittleFS.info(info);

  debugln("File system info:");

  debug("Total space:      ");
  debug(String(info.totalBytes));
  debugln("byte");

  debug("Total space used: ");
  debug(String(info.usedBytes));
  debugln("byte");

  debug("Total space free: ");
  debug(String(info.totalBytes - info.usedBytes));
  debugln("byte");
  debugln();

  Dir dir = LittleFS.openDir("/");
  while (dir.next()) {
    debug(dir.fileName());
    if (dir.fileSize()) {
      File file = dir.openFile("r");
      count++;
      size += file.size();
      for (int i = dir.fileName().length(); i < 17; i++) debug(" ");
      debugln(" " + String(file.size()));
      file.close();
    }
  }
  String countString = "   " + String(count) + " File(s)";
  debug(countString);
  for (int i = countString.length(); i < 17; i++) debug(" ");
  debugln(" " + String(size));
}

unsigned int Files::availableSpace() {
  FSInfo info;
  LittleFS.info(info);
  return (info.totalBytes - info.usedBytes);
}


void Scan::setup() {
  debugln("I2C Scanner Complete");
}

#define MAX_SCAN_DEVICES 6

void Scan::scan(Output* screen) {
  byte error, address;
  int nDevices;
  String devicesFound[MAX_SCAN_DEVICES];

  debugln("\nI2C Scanner");
  debugln("Scanning...");

  nDevices = 0;
  for (address = 1; address < 127; address++) {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    mutex->take();
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    mutex->give();

    if (error == 0) {
      debug("I2C device found at address 0x");
      if (address < 16)
        debug("0");
      debug(String(address, HEX));
      debugln("  !");

      if (nDevices < MAX_SCAN_DEVICES) {
        devicesFound[nDevices] = String(nDevices) + ". 0x" + String(address, HEX);
      }

      nDevices++;

    } else if (error == 4) {
      debug("Unknown error at address 0x");
      if (address < 16)
        debug("0");
      debugln(String(address, HEX));
    }
  }

  if (nDevices == 0)
    debugln("No I2C devices found\n");
  else
    debugln("done\n");
  screen->setScreen("I2c Scanner", "", devicesFound[0], devicesFound[1], devicesFound[2], devicesFound[3], devicesFound[4], devicesFound[5]);
}

#define HTML_BUFFER_SIZE 4000
static char html[HTML_BUFFER_SIZE];

HTMLBuilder::HTMLBuilder() {
  memset(html, 0, HTML_BUFFER_SIZE);
  index = 0;
}

void HTMLBuilder::print(const char* line) {
  unsigned int size = strlen(line);
  if ((size + index) < HTML_BUFFER_SIZE) {
    memcpy(&html[index], line, strlen(line));
    index += size;
  } else {
    debugln("ERROR: Building HTML exceeded buffer length");
  }
}

void HTMLBuilder::println() {
  char crlf[3] = "\r\n";
  print(crlf);
}

void HTMLBuilder::println(const char* line) {
  print(line);
  println();
}

void HTMLBuilder::print(String line) {
  char charArray[200];
  line.toCharArray(charArray, 200);
  print(charArray);
}

void HTMLBuilder::println(String line) {
  print(line);
  println();
}

void HTMLBuilder::print(int line) {
  char charArray[10];
  sprintf(charArray, "%d", line);
  print(charArray);
}

void HTMLBuilder::println(int line) {
  print(line);
  println();
}

char* HTMLBuilder::buffer() {
  return html;
}

unsigned int HTMLBuilder::length() {
  return index;
}
