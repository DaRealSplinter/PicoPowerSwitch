#include "powerserver.h"

#ifdef ETHERNET_SERVER

#ifndef ETHERNET
#error "Ethernet Module must be included"
#endif

#define BUFFER_SIZE 8192
#define HEADER_LENGTH 2048

void PowerServer::setup(Files* f) {
  files = f;
  for (int i = 0; i < NUM_DEVICES; i++) {
    headerStringOn[i] = String("GET /" + String(i + 1) + "/on");
    headerStringOff[i] = String("GET /" + String(i + 1) + "/off");
    headerStringMinOn[i] = String("GET /on/" + String(i + 1));
    headerStringMinOff[i] = String("GET /off/" + String(i + 1));
    headerStringMinStat[i] = String("GET /stat/" + String(i + 1));
  }

  headerStringMain = "GET /";
  headerStringConfig = "GET /configname";
  headerStringConfigIP = "GET /ipconfig";
  headerStringProcess = "GET /processname";
  headerStringProcessIP = "GET /ipprocess";
  headerStringServer = "GET /server";
  headerStringReboot = "GET /reboot";
  headerStringUpload = "GET /upload";
  headerStringUpgrade = "GET /upgrade";
  headerStringPostUpload = "POST /upload";
  headerStringPostUpgrade = "POST /upgrade";

  server.begin();
  println(PASSED, "ETHERNET Server Complete");
}

void PowerServer::sendPageBegin(HTMLBuilder* html, EthernetModule* ethernet, bool autoRefresh, int seconds) {
  // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
  // and a content-type so the client knows what's coming, then a blank line:
  html->println("HTTP/1.1 200 OK");
  html->println("Content-type:text/html");
  html->println("Connection: close");
  html->println();


  // Display the HTML web page
  html->println("<!DOCTYPE html><html>");
  html->print("<head>");
  if (autoRefresh) {
    html->print("<meta http-equiv=\"refresh\" content=\"");
    html->print(seconds);
    html->print("; url=http://");
    html->print(ethernet->getIPAddress().toString());
    html->println("/\"; name=\"viewport\" content=\"width=device-width, initial-scale=1\" >");
  }
  // CSS to style the on/off buttons
  html->println("<style>html { font-family: courier; font-size: 24px; display: inline-block; margin: 0px auto; text-align: center;}");
  html->println(".button { background-color: red; border: none; color: white; padding: 16px 40px;");
  html->println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
  html->println(".button2 {background-color: black;}");
  html->println(".button3 {background-color: grey;}");
  html->println(".button4 {background-color: green;}");
  html->println(".center {margin-left: auto; margin-right: auto;}");
  html->println("footer {");
  html->println("text-align: center;font-size: 16px;");
  html->println("padding: 3px;");
  html->println("background-color: MediumSeaGreen;");
  html->println("color: white;");
  html->println("}</style>");
  html->print("<title>");
  html->print(APP_NAME);
  html->println("</title></head>");
  html->print("<body><h1>");
  html->print(APP_NAME);
  html->println("</h1>");
}

void PowerServer::sendPageEnd(HTMLBuilder* html) {
  String versionString = "Ver. " + String(PROGRAM_NUMBER) + String(".") + String(PROGRAM_VERSION_MAJOR) + String(".") + String(PROGRAM_VERSION_MINOR);

  html->println("<footer><p>");
  html->println(APP_NAME);
  html->println("<br>" + versionString);
  html->println("<br>Build Date: " + String(compileDate) + " Time: " + String(compileTime));
  html->println("<br>Author: John J. Gavel<br>");
  html->println("</p></footer>");
  html->println("</body></html>");
  html->println();
}

void PowerServer::sendPowerPage(HTMLBuilder* html, EthernetModule* ethernet, Temperature* temperature, EEpromMemory* memory, Gpio* gpio, bool fastRefresh) {
  if (fastRefresh)
    sendPageBegin(html, ethernet, true, 1);
  else sendPageBegin(html, ethernet, true);
  if (temperature->validTemperature()) {
    html->print("<p>Temperature ");
    html->print(temperature->getTemperature());
    html->println(" F</p>");
  }

  html->print("<table class=\"center\">");
  for (byte i = 0; i < NUM_DEVICES; i++) {
    html->print("<tr><td><p>Power ");
    html->print(i + 1);
    html->print(". </td><td>");
    html->print(memory->getDeviceName(i));
    html->print("</td></p></td><td><a href=\"/");
    html->print(i + 1);
    if (gpio->getRelay(i) == HIGH) {
      html->println("/off\"><button class=\"button\">ON</button></a></td></tr></p>");
    } else {
      html->println("/on\"><button class=\"button2 button\">OFF</button></a></td></tr>");
    }
  }
  html->println("</table>");
  html->println("<table class=\"center\"><tr><td><a href=\"/configname\">Configure Devices</a></td>");
  html->println("<td><a href=\"/server\">Server Control</a></td></tr></table>");
  sendPageEnd(html);
  client.write(html->buffer(), html->length());
}

void PowerServer::sendConfigIPPage(HTMLBuilder* html, EthernetModule* ethernet, EEpromMemory* memory) {
  sendPageBegin(html, ethernet);
  html->println("<form action=\"/ipprocess\" method=\"GET\">");

  bool isDhcp = memory->mem.mem.isDHCP;
  html->println("<fieldset><legend>Select IP Address Source</legend>");
  html->print("<input type=\"radio\" id=\"dhcp0\" name=\"dhcp\" value=\"0\"");
  html->print((!isDhcp) ? " checked />" : " />");
  html->println("<label for=\"dhcp0\">Static IP</label>");
  html->print("<input type=\"radio\" id=\"dhcp1\" name=\"dhcp\" value=\"1\"");
  html->print((isDhcp) ? " checked />" : " />");
  html->println("<label for=\"dhcp1\">DHCP</label>");
  html->println("</fieldset>");

  html->print("<table class=\"center\">");

  byte* address = memory->mem.mem.ipAddress;
  html->print("<tr><td>IP Address</td>");
  html->print("<td><input type=\"text\" maxlength=\"3\" size=\"3\" pattern=\"[^\\s]+\" value=\"" + String(address[0]) + "\" name=\"ip0\"\"></td>");
  html->print("<td><input type=\"text\" maxlength=\"3\" size=\"3\" pattern=\"[^\\s]+\" value=\"" + String(address[1]) + "\" name=\"ip1\"\"></td>");
  html->print("<td><input type=\"text\" maxlength=\"3\" size=\"3\" pattern=\"[^\\s]+\" value=\"" + String(address[2]) + "\" name=\"ip2\"\"></td>");
  html->print("<td><input type=\"text\" maxlength=\"3\" size=\"3\" pattern=\"[^\\s]+\" value=\"" + String(address[3]) + "\" name=\"ip3\"\"></td></tr>");

  address = memory->mem.mem.subnetMask;
  html->print("<tr><td>Subnet Mask</td>");
  html->print("<td><input type=\"text\" maxlength=\"3\" size=\"3\" pattern=\"[^\\s]+\" value=\"" + String(address[0]) + "\" name=\"sm0\"\"></td>");
  html->print("<td><input type=\"text\" maxlength=\"3\" size=\"3\" pattern=\"[^\\s]+\" value=\"" + String(address[1]) + "\" name=\"sm1\"\"></td>");
  html->print("<td><input type=\"text\" maxlength=\"3\" size=\"3\" pattern=\"[^\\s]+\" value=\"" + String(address[2]) + "\" name=\"sm2\"\"></td>");
  html->print("<td><input type=\"text\" maxlength=\"3\" size=\"3\" pattern=\"[^\\s]+\" value=\"" + String(address[3]) + "\" name=\"sm3\"\"></td></tr>");

  address = memory->mem.mem.gatewayAddress;
  html->print("<tr><td>Gateway Address</td>");
  html->print("<td><input type=\"text\" maxlength=\"3\" size=\"3\" pattern=\"[^\\s]+\" value=\"" + String(address[0]) + "\" name=\"ga0\"\"></td>");
  html->print("<td><input type=\"text\" maxlength=\"3\" size=\"3\" pattern=\"[^\\s]+\" value=\"" + String(address[1]) + "\" name=\"ga1\"\"></td>");
  html->print("<td><input type=\"text\" maxlength=\"3\" size=\"3\" pattern=\"[^\\s]+\" value=\"" + String(address[2]) + "\" name=\"ga2\"\"></td>");
  html->print("<td><input type=\"text\" maxlength=\"3\" size=\"3\" pattern=\"[^\\s]+\" value=\"" + String(address[3]) + "\" name=\"ga3\"\"></td></tr>");

  address = memory->mem.mem.dnsAddress;
  html->print("<tr><td>DNS Address</td>");
  html->print("<td><input type=\"text\" maxlength=\"3\" size=\"3\" pattern=\"[^\\s]+\" value=\"" + String(address[0]) + "\" name=\"da0\"\"></td>");
  html->print("<td><input type=\"text\" maxlength=\"3\" size=\"3\" pattern=\"[^\\s]+\" value=\"" + String(address[1]) + "\" name=\"da1\"\"></td>");
  html->print("<td><input type=\"text\" maxlength=\"3\" size=\"3\" pattern=\"[^\\s]+\" value=\"" + String(address[2]) + "\" name=\"da2\"\"></td>");
  html->print("<td><input type=\"text\" maxlength=\"3\" size=\"3\" pattern=\"[^\\s]+\" value=\"" + String(address[3]) + "\" name=\"da3\"\"></td></tr>");

  html->print("</table><table class=\"center\"><tr><td><button type=\"submit\" class=\"button4 button\">Submit</button></td>");
  html->println("<td><button type=\"reset\" class=\"button\">Reset</button></td>");
  html->println("<td><a href=\"/\"><button type=\"button\" class=\"button2 button\">Cancel</button></a></td></tr>");
  html->println("</table>");
  html->println("</form>");
  sendPageEnd(html);
  client.write(html->buffer(), html->length());
}

void PowerServer::sendConfigPage(HTMLBuilder* html, EthernetModule* ethernet, EEpromMemory* memory) {
  sendPageBegin(html, ethernet);
  html->println("<form action=\"/processname\" method=\"GET\">");
  html->print("<table class=\"center\">");
  for (byte i = 0; i < NUM_DEVICES; i++) {
    html->print("<tr><td><p>Power ");
    html->print(i + 1);
    html->print(". ");
    html->print(memory->getDeviceName(i));
    html->print("</td><td>");
    html->print("<input type=\"text\" maxlength=\"15\" pattern=\"[^\\s]+\" value=\"");
    html->print(memory->getDeviceName(i));
    html->print("\" name=\"");
    html->print(i + 1);
    html->print("\">");
    html->println("</td></tr>");
  }
  html->print("<tr><td><button type=\"submit\" class=\"button4 button\">Submit</button></td>");
  html->println("<td><button type=\"reset\" class=\"button\">Reset</button></td>");
  html->println("<td><a href=\"/\"><button type=\"button\" class=\"button2 button\">Cancel</button></a></td></tr>");
  html->println("</table>");
  html->println("<p>(No whitespace, max length is 15 characters)</p>");
  html->println("</form>");
  sendPageEnd(html);
  client.write(html->buffer(), html->length());
}

void PowerServer::sendServerPage(HTMLBuilder* html, EthernetModule* ethernet, EEpromMemory* memory, Gpio* gpio) {
  sendPageBegin(html, ethernet);
  String versionString = "Ver. " + String(PROGRAM_NUMBER) + String(".") + String(PROGRAM_VERSION_MAJOR) + String(".") + String(PROGRAM_VERSION_MINOR);

  html->println("<h2>");
  html->println(APP_NAME);
  html->println("</h2><table class=\"center\"> <tr><td>" + versionString + "</td></tr>");
  html->println("<tr><td>Build Date: " + String(compileDate) + " Time: " + String(compileTime) + "</td></tr>");
  html->println("<tr><td>Author: John J. Gavel</td></tr></table>");
  html->println();

  html->println("<h2>MAC Address</h2><table class=\"center\"> <tr><td>MAC Address:</td><td>" + String(memory->mem.mem.macAddress[0], HEX) + String(":") + String(memory->mem.mem.macAddress[1], HEX) + String(":") + String(memory->mem.mem.macAddress[2], HEX) + String(":") + String(memory->mem.mem.macAddress[3], HEX) + String(":") + String(memory->mem.mem.macAddress[4], HEX) + String(":") + String(memory->mem.mem.macAddress[5], HEX) + "</td></tr></table>");
  html->println("<h2>IP Configuration</h2><table class=\"center\">");
  html->print("<tr><td>IP Configuration:</td><td>" + String((memory->mem.mem.isDHCP) ? "DHCP" : "Static") + "</td></tr>");
  IPAddress ipAddress = ethernet->getIPAddress();
  html->println("<tr><td>IP Address:</td><td>" + String(ipAddress[0]) + String(".") + String(ipAddress[1]) + String(".") + String(ipAddress[2]) + String(".") + String(ipAddress[3]) + "</td></tr>");
  ipAddress = ethernet->getDNS();
  html->println("<tr><td>DNS Server:</td><td>" + String(ipAddress[0]) + String(".") + String(ipAddress[1]) + String(".") + String(ipAddress[2]) + String(".") + String(ipAddress[3]) + "</td></tr>");
  ipAddress = ethernet->getSubnetMask();
  html->println("<tr><td>Subnet Mask:</td><td>" + String(ipAddress[0]) + String(".") + String(ipAddress[1]) + String(".") + String(ipAddress[2]) + String(".") + String(ipAddress[3]) + "</td></tr>");
  ipAddress = ethernet->getGateway();
  html->println("<tr><td>Gateway:</td><td>" + String(ipAddress[0]) + String(".") + String(ipAddress[1]) + String(".") + String(ipAddress[2]) + String(".") + String(ipAddress[3]) + "</td></tr></table>");

  html->println("<h2>Power Status</h2><table class=\"center\">");
  for (int i = 0; i < memory->getNumberOfDevices(); i++) {
    html->print("<tr><td>Power </td><td>");
    html->print(String(i + 1));
    html->print(" </td><td>");
    html->print(String(memory->getDeviceName(i)));
    html->println("</td>");
    if (gpio->getRelay(i) == HIGH) {
      html->println("<td>ON</td></tr>");
    } else {
      html->println("<td>OFF</td></tr>");
    }
  }
  html->println("</table>");
  html->println("<h2>Hardware Status</h2><table class=\"center\">");
  html->println("<tr><td>Pico</td><td>" + String((gpio->getOnline(0)) ? "ON" : "OFF") + "</td>");
  html->println("<tr><td>Display</td><td>" + String((gpio->getOnline(1)) ? "ON" : "OFF") + "</td>");
  html->println("<tr><td>GPIO Ex</td><td>" + String((gpio->getOnline(2)) ? "ON" : "OFF") + "</td>");
  html->println("<tr><td>Temperature</td><td>" + String((gpio->getOnline(3)) ? "ON" : "OFF") + "</td>");
  html->println("<tr><td>Ethernet</td><td>" + String((gpio->getOnline(4)) ? "ON" : "OFF") + "</td>");
  html->println("<tr><td>EEPROM</td><td>" + String((gpio->getOnline(5)) ? "ON" : "OFF") + "</td>");
  html->println("</table>");
  html->println("<tr><a href=\"/ipconfig\">Configure IP Addresses</a></tr>");
  html->println("<br><tr><a href=\"/upgrade\">Upgrade the Power Switch</a></tr>");
  html->println("<table class=\"center\">");
  html->println("<td><a href=\"/\"><button type=\"button\" class=\"button2 button\">Cancel</button></a></td></tr>");
  html->println("</table>");
  html->println("<table class=\"center\"><a href=\"/reboot\"><button class=\"button\">REBOOT</button></a></table>");
  sendPageEnd(html);
  client.write(html->buffer(), html->length());
}

static char* subStringAfterQuestion(char* action) {
  char* returnString = NULL;
  for (int i = 0; i < HEADER_LENGTH; i++)
    if (action[i] == '?') returnString = &action[i + 1];
  return returnString;
}

void PowerServer::sendProcessHTMLPage(HTMLBuilder* html, EthernetModule* ethernet, String action, unsigned int timeout) {
  sendPageBegin(html, ethernet, true, timeout);
  html->print("<p>");
  html->print(action);
  html->println("</p>");
  sendPageEnd(html);
  client.write(html->buffer(), html->length());
}

void PowerServer::sendProcessPage(HTMLBuilder* html, EthernetModule* ethernet, EEpromMemory* memory, char* action) {
  char* token;
  char* processString;
  processString = subStringAfterQuestion(action);
  if (processString) {
    token = strtok(processString, "=&");
    for (int i = 0; i < NUM_DEVICES; i++) {
      String number = String(token);
      int index = number.toInt() - 1;
      token = strtok(NULL, "=&");
      memory->setDeviceName(index, token, strlen(token));
      token = strtok(NULL, "=&");
    }
  }
  sendProcessHTMLPage(html, ethernet, "Processing Device Names....");
}

void PowerServer::sendProcessIPPage(HTMLBuilder* html, EthernetModule* ethernet, EEpromMemory* memory, char* action) {
  char* token;
  char* processString;
  processString = subStringAfterQuestion(action);
  if (processString) {
    token = strtok(processString, "=&");
    token = strtok(NULL, "=&");
    memory->mem.mem.isDHCP = (atoi(token));
    token = strtok(NULL, "=&");
    token = strtok(NULL, "=&");
    memory->mem.mem.ipAddress[0] = (atoi(token));
    token = strtok(NULL, "=&");
    token = strtok(NULL, "=&");
    memory->mem.mem.ipAddress[1] = (atoi(token));
    token = strtok(NULL, "=&");
    token = strtok(NULL, "=&");
    memory->mem.mem.ipAddress[2] = (atoi(token));
    token = strtok(NULL, "=&");
    token = strtok(NULL, "=&");
    memory->mem.mem.ipAddress[3] = (atoi(token));

    token = strtok(NULL, "=&");
    token = strtok(NULL, "=&");
    memory->mem.mem.subnetMask[0] = (atoi(token));
    token = strtok(NULL, "=&");
    token = strtok(NULL, "=&");
    memory->mem.mem.subnetMask[1] = (atoi(token));
    token = strtok(NULL, "=&");
    token = strtok(NULL, "=&");
    memory->mem.mem.subnetMask[2] = (atoi(token));
    token = strtok(NULL, "=&");
    token = strtok(NULL, "=&");
    memory->mem.mem.subnetMask[3] = (atoi(token));

    token = strtok(NULL, "=&");
    token = strtok(NULL, "=&");
    memory->mem.mem.gatewayAddress[0] = (atoi(token));
    token = strtok(NULL, "=&");
    token = strtok(NULL, "=&");
    memory->mem.mem.gatewayAddress[1] = (atoi(token));
    token = strtok(NULL, "=&");
    token = strtok(NULL, "=&");
    memory->mem.mem.gatewayAddress[2] = (atoi(token));
    token = strtok(NULL, "=&");
    token = strtok(NULL, "=&");
    memory->mem.mem.gatewayAddress[3] = (atoi(token));

    token = strtok(NULL, "=&");
    token = strtok(NULL, "=&");
    memory->mem.mem.dnsAddress[0] = (atoi(token));
    token = strtok(NULL, "=&");
    token = strtok(NULL, "=&");
    memory->mem.mem.dnsAddress[1] = (atoi(token));
    token = strtok(NULL, "=&");
    token = strtok(NULL, "=&");
    memory->mem.mem.dnsAddress[2] = (atoi(token));
    token = strtok(NULL, "=&");
    token = strtok(NULL, "=&");
    memory->mem.mem.dnsAddress[3] = (atoi(token));
    memory->breakSeal();
  }
  sendProcessHTMLPage(html, ethernet, "Processing IP Configuration....\nREBOOT Module for changes to have effect.");
}

static char fileBuffer[BUFFER_SIZE];
void PowerServer::sendFile(File* file) {
  memset(fileBuffer, 0, BUFFER_SIZE);
  unsigned int remainder = file->size() % BUFFER_SIZE;
  unsigned int loops = file->size() / BUFFER_SIZE;
  unsigned int bytes = 0;
  for (unsigned int i = 0; i < loops; i++) {
    bytes = file->readBytes(fileBuffer, BUFFER_SIZE);
    bytes = client.write(fileBuffer, bytes);
    memset(fileBuffer, 0, BUFFER_SIZE);
  }
  bytes = file->readBytes(fileBuffer, remainder);
  bytes = client.write(fileBuffer, bytes);
}

bool PowerServer::receiveFile(Watchdog* watchdog, File* file, unsigned int bytes) {
  Task timeout;
  unsigned int total = 0;

  timeout.setRefresh(1000);
  memset(fileBuffer, 0, BUFFER_SIZE);
  unsigned int receivedBytes = 0;
  while ((total < bytes) && !timeout.run()) {
    receivedBytes = client.read((uint8_t*)fileBuffer, BUFFER_SIZE);
    if ((total + receivedBytes) > bytes)
      receivedBytes = bytes - total;
    receivedBytes = file->write(fileBuffer, receivedBytes);
    total += receivedBytes;
    if (receivedBytes > 0) timeout.reset();
    memset(fileBuffer, 0, BUFFER_SIZE);
    watchdog->petWatchdog();
    delay(10);
  }
  //print("Received File: ");
  //print(String(total));
  //print("/");
  //println(String(bytes));
  return (total == bytes);
}

void PowerServer::sendUploadPage(HTMLBuilder* html, EthernetModule* ethernet, String description) {
  sendPageBegin(html, ethernet);
  html->println(description);
  html->println("<form method=\"post\" enctype=\"multipart/form-data\">");
  html->println("<label for=\"file\">File</label>");
  html->println("<input id=\"file\" name=\"file\" type=\"file\" />");
  html->println("<button>Upload</button>");
  html->println("</form><br>");
  html->println("<td><a href=\"/server\"><button type=\"button\" class=\"button2 button\">Cancel</button></a></td></tr><br>");
  sendPageEnd(html);
  client.write(html->buffer(), html->length());
}

void PowerServer::sendErrorPage(HTMLBuilder* html, EthernetModule* ethernet) {
  sendPageBegin(html, ethernet, true, 5);
  html->println("<section>");
  html->println("  <div class=\"container\">");
  html->println("   <div><img class=\"image\" src=\"/errorimg.png\" alt=\"\"></div>");
  html->println("  </div>");
  html->println("  </div>");
  html->println("</section>");
  sendPageEnd(html);
  client.write(html->buffer(), html->length());
}

void PowerServer::processHeader(EthernetModule* ethernet, EEpromMemory* memory, Gpio* gpio, Temperature* temperature, char* action) {
  bool getPowerPage = false;
  bool minimalist = false;
  HTMLBuilder html;
  // if the current line is blank, you got two newline characters in a row.
  // that's the end of the client HTTP request, so send a response:
  //print("<");
  //print(action);
  //println(">");
  // turns the GPIOs on and off
  for (byte i = 0; i < NUM_DEVICES; i++) {
    if (headerStringOn[i].equals(String(action))) {
      if (gpio->getRelay(i) == LOW)
        gpio->setCommand(i);
      getPowerPage = true;
      //print("Get Device " + String(i + 1) + " ON - ");
    } else if (headerStringOff[i].equals(String(action))) {
      if (gpio->getRelay(i) == HIGH)
        gpio->setCommand(i);
      getPowerPage = true;
      //print("Get Device " + String(i + 1) + " OFF - ");
    } else if (headerStringMinOn[i].equals((String(action)))) {
      minimalist = true;
      if (gpio->getRelay(i) == LOW)
        gpio->setCommand(i);
      html.println("on");
    } else if (headerStringMinOff[i].equals((String(action)))) {
      minimalist = true;
      if (gpio->getRelay(i) == HIGH)
        gpio->setCommand(i);
      html.println("off");
    } else if (headerStringMinStat[i].equals((String(action)))) {
      minimalist = true;
      if (gpio->getRelay(i) == LOW)
        html.println("off");
      else
        html.println("on");
    }
  }
  if ((headerStringMain.equals(String(action))) || getPowerPage) {
    //print("Get Power ");
    sendPowerPage(&html, ethernet, temperature, memory, gpio, getPowerPage);
  } else if (headerStringConfig.equals(String(action))) {
    //print("Get Config ");
    sendConfigPage(&html, ethernet, memory);
  } else if (headerStringConfigIP.equals(String(action))) {
    //print("Get Config IP");
    sendConfigIPPage(&html, ethernet, memory);
  } else if ((String(action)).startsWith(headerStringProcess)) {
    //print("Get Process ");
    sendProcessPage(&html, ethernet, memory, action);
  } else if ((String(action)).startsWith(headerStringProcessIP)) {
    //print("Get IPProcess ");
    sendProcessIPPage(&html, ethernet, memory, action);
  } else if (headerStringReboot.equals(String(action))) {
    //print("Get Reboot ");
    sendPageBegin(&html, ethernet, true, 6);
    html.println("<p>Rebooting.....</p>");
    sendPageEnd(&html);
    client.write(html.buffer(), html.length());
    client.stop();
    rp2040.reboot();
  } else if (headerStringServer.equals(String(action))) {
    //print("Get Server ");
    sendServerPage(&html, ethernet, memory, gpio);
  } else if (headerStringUpload.equals(String(action))) {
    //print("Get Upload");
    sendUploadPage(&html, ethernet);
  } else if (headerStringUpgrade.equals(String(action))) {
    //print("Get Upgrade");
    sendUploadPage(&html, ethernet, "<h2>OTA Upgrade<h2/>");
  } else if (minimalist) {
    client.write(html.buffer(), html.length());
  } else if ((String(action)).startsWith(headerStringMain)) {
    char* fileName = &action[4];
    //print("Get File " + String(fileName));
    File file = files->getFile(fileName);
    if (file) {
      sendFile(&file);
    } else {
      println(ERROR, "SERVER: " + String(fileName) + " not found!");
      sendErrorPage(&html, ethernet);
    }
    file.close();
  } else {
    String unknown = "UNKNOWN Connection to Server <" + String(action) + ">";
    println(WARNING, unknown);
  }
  //println(" - Complete");
}

static const char* contentLength = "Content-Length: ";
static const char* contentType = "Content-Type: multipart/form-data; boundary=----";
static const char* contentDisposition = "Content-Disposition: ";
static const char* fileNameToken = " :;=\"";

typedef enum {
  POST_UPLOAD,
  CHECK_BOUNDARY,
  FORM_DISP,
  STRING1,
  STRING2,
  FILE_CONTENTS,
  UPLOAD_DONE,
  ERROR_STATE
} POST_STATE;

static char postBuffer[HEADER_LENGTH];
#define WORKING_LENGTH 128

void PowerServer::processPost(EthernetModule* ethernet, Watchdog* watchdog, char* action) {
  HTMLBuilder html;
  bool upgradeFileFlag = headerStringPostUpgrade.equals(String(action));
  bool uploadFileFlag = headerStringPostUpload.equals(String(action));

  memset(postBuffer, 0, HEADER_LENGTH);
  if (upgradeFileFlag || uploadFileFlag) {
    char c;
    unsigned int count = 0;
    unsigned int fileLength = 0;
    char boundary[WORKING_LENGTH];
    char fileName[WORKING_LENGTH];
    char* workingString;
    POST_STATE state = POST_UPLOAD;
    File uploadFile;

    memset(boundary, 0, WORKING_LENGTH);
    memset(fileName, 0, WORKING_LENGTH);

    while (client.available() && (state != UPLOAD_DONE)) {
      switch (state) {
        case POST_UPLOAD:
          c = client.read();
          if (c == '\n') {
            if (strncmp(postBuffer, contentLength, strlen(contentLength)) == 0) {
              fileLength = atoi(&postBuffer[strlen(contentLength)]);
            } else if (strncmp(postBuffer, contentType, strlen(contentType)) == 0) {
              strncpy(boundary, &postBuffer[strlen(contentType)], WORKING_LENGTH);
              state = CHECK_BOUNDARY;
            }
            memset(postBuffer, 0, HEADER_LENGTH);
            count = 0;
          } else if (count < HEADER_LENGTH) {
            postBuffer[count++] = c;
          } else {
            state = ERROR_STATE;
          }
          break;
        case CHECK_BOUNDARY:
          c = client.read();
          if (c == '\n') {
            if (strncmp(&postBuffer[6], boundary, strlen(boundary)) == 0) {
              fileLength = fileLength - ((strlen(postBuffer) * 2) + 4);
              state = FORM_DISP;
            }
            memset(postBuffer, 0, HEADER_LENGTH);
            count = 0;
          } else if (count < HEADER_LENGTH) {
            postBuffer[count++] = c;
          } else {
            state = ERROR_STATE;
          }
          break;
        case FORM_DISP:
          c = client.read();
          if (c == '\n') {
            if (strncmp(postBuffer, contentDisposition, strlen(contentDisposition)) == 0) {
              fileLength = fileLength - strlen(postBuffer) - 1;
              workingString = strtok(postBuffer, fileNameToken);
              while ((workingString != NULL) && strncmp(workingString, "filename", 8) != 0) {
                workingString = strtok(NULL, fileNameToken);
              }
              if (workingString != NULL) {
                workingString = strtok(NULL, fileNameToken);
                if (workingString != NULL) {
                  strncpy(fileName, workingString, WORKING_LENGTH);
                  state = STRING1;
                } else {
                  state = ERROR_STATE;
                }
              } else {
                state = ERROR_STATE;
              }
            }
            memset(postBuffer, 0, HEADER_LENGTH);
            count = 0;
          } else if (count < HEADER_LENGTH) {
            postBuffer[count++] = c;
          } else {
            state = ERROR_STATE;
          }
          break;
        case STRING1:
          c = client.read();
          if (c == '\n') {
            fileLength = fileLength - strlen(postBuffer) - 1;
            state = STRING2;
            memset(postBuffer, 0, HEADER_LENGTH);
            count = 0;
          } else if (count < HEADER_LENGTH) {
            postBuffer[count++] = c;
          } else {
            state = ERROR_STATE;
          }
          break;
        case STRING2:
          c = client.read();
          if (c == '\n') {
            fileLength = fileLength - strlen(postBuffer) - 3;
            if (fileLength < files->availableSpace()) {
              if (upgradeFileFlag) uploadFile = files->writeFile(UPGRADE_FILE_NAME);
              else uploadFile = files->writeFile(fileName);
              if (uploadFile) {
                state = FILE_CONTENTS;
              } else {
                state = ERROR_STATE;
              }
            } else {
              println(WARNING, "NOT ENOUGH SPACE FOR FILE: " + String(fileName) + String(" Size: ") + String(fileLength) + String("/") + String(files->availableSpace()));
              state = ERROR_STATE;
            }
            memset(postBuffer, 0, HEADER_LENGTH);
            count = 0;
          } else if (count < HEADER_LENGTH) {
            postBuffer[count++] = c;
          } else {
            state = ERROR_STATE;
          }
          break;
        case FILE_CONTENTS:
          if (receiveFile(watchdog, &uploadFile, fileLength)) state = UPLOAD_DONE;
          else state = ERROR_STATE;
          uploadFile.close();
          break;
        case ERROR_STATE:
        default:
          c = client.read();
          break;
      }
    }
    if (uploadFileFlag) {
      if (state == UPLOAD_DONE)
        sendProcessHTMLPage(&html, ethernet, "Processing File Upload....");
      else
        sendProcessHTMLPage(&html, ethernet, "Processing File Upload FAILED....");
    } else if (upgradeFileFlag) {
      if (state == UPLOAD_DONE) {
        sendProcessHTMLPage(&html, ethernet, "Processing File Upgrade....", 20);
        UPGRADE_SYSTEM();
        watchdog->reboot();
      } else
        sendProcessHTMLPage(&html, ethernet, "Processing File Upgrade FAILED....");
    } else {
      sendProcessHTMLPage(&html, ethernet, "Processing File POST FAILED....");
    }
  } else {
    sendErrorPage(&html, ethernet);
  }
}

static char headerBuffer[HEADER_LENGTH];

void PowerServer::loop(EthernetModule* ethernet, EEpromMemory* memory, Gpio* gpio, Temperature* temperature, Watchdog* watchdog) {
  unsigned long startTime;
  const unsigned long timeoutTime = 1000;
  bool actionRcv = false;

  memset(headerBuffer, 0, HEADER_LENGTH);
  unsigned int headerIndex = 0;
  client = server.available();
  if (client) {  // If a new client connects,
    client.setTimeout(timeoutTime);
    startTime = millis();
    while (client.connected() && ((millis() - startTime) < timeoutTime)) {  // loop while the client's connected
      if (client) {                                                         // if there's bytes to read from the client,
        char c = client.read();                                             // read a byte
        //print(String(c));

        if ((headerIndex <= 4) || (strncmp("GET ", headerBuffer, 4) == 0) || (strncmp("POST", headerBuffer, 4) == 0)) {
          if (headerIndex < 5) {
            headerBuffer[headerIndex++] = c;
            actionRcv = false;
          } else if ((actionRcv == false) && (c != ' ')) {
            headerBuffer[headerIndex++] = c;
          } else {
            actionRcv = true;
          }
        }

        if ((c == '\n') && (strncmp("GET", headerBuffer, 3) == 0)) {  // if the byte is a newline character
          processHeader(ethernet, memory, gpio, temperature, headerBuffer);
          break;
        }
        if ((c == '\n') && (strncmp("POST", headerBuffer, 4) == 0)) {  // if the byte is a newline character
          processPost(ethernet, watchdog, headerBuffer);
          break;
        }
        if (c == '\n') {
          // Close the connection
          break;
        }
      } else delay(5);
    }
    client.flush();
    client.stop();
    //println("Client disconnected.\n");
  }
  //server.statusreport();
}
#else
#pragma GCC warning "No Server Module Included"

void PowerServer::setup(Files* f){};
void PowerServer::loop(EthernetModule* ethernet, EEpromMemory* memory, Gpio* gpio, Temperature* temperature, Watchdog* watchdog){};
#endif
