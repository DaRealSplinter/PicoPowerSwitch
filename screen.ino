#include "screen.h"

#ifdef SCREEN
#include "bitmap.h"

void Screen::setup(Gpio* gpio) {
  byte error;

  mutex->take();

  Wire.beginTransmission(SCREEN_ADDRESS);
  error = Wire.endTransmission();

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS) || (error != 0)) {
    println(ERROR, "SSD1306 Display Not Connected");
  }
  // Clear the buffer
  display.clearDisplay();
  mutex->give();
  setScreen(JAXSON);
  //String versionString = "Ver. " + String(PROGRAM_NUMBER) + String(".") + String(PROGRAM_VERSION_MAJOR) + String(".") + String(PROGRAM_VERSION_MINOR);
  //setScreen(APP_NAME, versionString, "By John J. Gavel", "", "Initializing");
  gpio->setOnline(SSD1306, (error == 0));
  println((error == 0) ? PASSED : FAILED, "Screen Complete");
}

void Screen::beginScreen() {
  mutex->take();
  reset();
  display.clearDisplay();
  display.setCursor(0, 0);  // Start at top-left corner
  display.setTextSize(1);   // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE);
  mutex->give();
}

void Screen::printScreen(String line) {
  mutex->take();
  display.print(line);
  mutex->give();
}

void Screen::printLnScreen(String line) {
  mutex->take();
  display.println(line);
  mutex->give();
}

void Screen::endScreen() {
  mutex->take();
  display.display();
  mutex->give();
}
void Screen::setScreen(BITMAP bitmap) {
  if (bitmap < BITMAP_LENGTH) {
    beginScreen();
    mutex->take();
    display.drawBitmap(0, 0, bitmap_allArray[bitmap], 128, 64, WHITE);
    display.display();
    mutex->give();
    setRefresh(5000);
  }
}

void Screen::setScreen(BITMAP bitmap, String caption) {
  setScreen(bitmap);
  mutex->take();
  display.setCursor(0, 56);  // Start at top-left corner
  int center = (20 - caption.length()) / 2;
  for (int i = 0; i <= center; i++) display.print(" ");
  display.print(caption);
  display.display();
  mutex->give();
}

void Screen::setScreen(String line1, String line2, String line3, String line4, String line5, String line6, String line7, String line8) {
  beginScreen();
  printLnScreen(line1);
  printLnScreen(line2);
  printLnScreen(line3);
  printLnScreen(line4);
  printLnScreen(line5);
  printLnScreen(line6);
  printLnScreen(line7);
  printLnScreen(line8);
  endScreen();
  setRefresh(5000);
}

#define SCREEN_TAB "   "

void Screen::loop(String progName, Temperature* temperature, Gpio* gpio, EthernetModule* ethernet) {
  if (run()) {
    setRefresh(1000);
    beginScreen();
    printLnScreen(progName);
    String versionString = "Ver. " + String(PROGRAM_NUMBER) + String(".") + String(PROGRAM_VERSION_MAJOR) + String(".") + String(PROGRAM_VERSION_MINOR);
    printLnScreen(versionString);

#ifdef TEMPERATURE
    if (temperature->validTemperature()) {
      printScreen("Temperature: ");
      printScreen(String(temperature->getTemperature()));
      printLnScreen(" F");
    } else {
      printLnScreen();
    }
#endif
#ifdef ETHERNET
    if (ethernet->linkStatus()) {
      IPAddress ipAddress = ethernet->getIPAddress();
      printScreen(String(ipAddress[0]));
      printScreen(".");
      printScreen(String(ipAddress[1]));
      printScreen(".");
      printScreen(String(ipAddress[2]));
      printScreen(".");
      printLnScreen(String(ipAddress[3]));
    } else {
      printLnScreen();
    }
#endif
    printLnScreen();
#ifdef GPIO
    for (byte i = 0; i < NUM_DEVICES; i++) {
      printScreen(String(i + 1));
      printScreen(SCREEN_TAB);
    }
    printLnScreen();
    for (byte i = 0; i < NUM_DEVICES; i++) {
      if (gpio->getRelay(i) == false)
        printScreen(" ");
      else
        printScreen("*");
      printScreen(SCREEN_TAB);
    }
    printLnScreen();
#endif
    endScreen();
  }
}

#else
#pragma GCC warning "No Display Module Included"
void Screen::setup(Gpio* gpio){};
void Screen::loop(String progName, Temperature* temperature, Gpio* gpio, EthernetModule* ethernet){};
void Screen::setScreen(String line1, String line2, String line3, String line4, String line5, String line6, String line7, String line8){};
void Screen::setScreen(BITMAP bitmap){};
void Screen::setScreen(BITMAP bitmap, String caption){};
#endif
