#ifndef __SCREEN
#define __SCREEN

//#define SSD1306_NO_SPLASH
#include <Adafruit_SSD1306.h>
#include "util.h"
#include "temperature.h"
#include "gpio.h"
#include "ethernetmodule.h"

#define SCREEN_LINES 8

#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library.
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...
#define OLED_RESET -1        // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C  ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

class Screen : public Output, protected Task {
public:
  Screen(Mutex* wireMutex)
    : mutex(wireMutex), display(Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET)){};
  void setup(Gpio* gpio);
  void loop(String progName, Temperature* temperature, Gpio* gpio, EthernetModule* ethernet);
  void beginScreen();
  void printScreen(String line);
  void printLnScreen(String line);
  void printLnScreen() {
    printLnScreen("");
  };
  void endScreen();
  void setScreen(String line1 = "", String line2 = "", String line3 = "", String line4 = "", String line5 = "", String line6 = "", String line7 = "", String line8 = "");
  void setScreen(BITMAP bitmap);
  void setScreen(BITMAP bitmap, String caption);
private:
  Mutex* mutex;
  Adafruit_SSD1306 display;
};

#endif
