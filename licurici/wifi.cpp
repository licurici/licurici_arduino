#include "wifi.h"

char ssid[] = "";
char pass[] = "";

int keyIndex = 0;             // your network key Index number (needed only for WEP)
int status = WL_IDLE_STATUS;  //connection status
WiFiServer server(80);        //server socket
WiFiClient client = server.available();

String strAction = "";
int actionReadIndex = 0;

int webReadInt() {
  if (actionReadIndex > strAction.length()) {
    return 0;
  }

  int pos = strAction.indexOf('/', actionReadIndex);
  auto value = strAction.substring(actionReadIndex, pos);

  client.print("-> ");
  client.println(value);

  actionReadIndex = pos + 1;

  return value.toInt();
}

void webPrintStr(const char message[]) {
  client.print(message);
}

void webPrintInt(int value) {
  client.print(value);
}

int setup_WiFi() {
  if (enable_WiFi() != 0) {
    return;
  }

  connect_WiFi();

  server.begin();
  printWifiStatus();
}

void loop_WiFi(FlushReport report, PerformAction performAction) {
  client = server.available();

  if (client) {
    printWEB(report, performAction);
  }
}


void printWifiStatus() {
  // print the SSID of the network you're attached to:

  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");

  Serial.print("To see this page in action, open a browser to http://");
  Serial.println(ip);
}

int enable_WiFi() {
  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    return 1;
  }

  String fv = WiFi.firmwareVersion();
  if (fv < "1.0.0") {
    Serial.println("Please upgrade the firmware");
    return 2;
  }

  return 0;
}

int connect_WiFi() {
  int counts = 0;

  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) {
    counts++;
    Serial.print("(");
    Serial.print(counts);
    Serial.print(") ");
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);

    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);

    if (counts >= 5) {
      return 1;
    }
  }

  return 0;
}

void readAction(String *action) {
  while (client.connected()) {
    if (!client.available()) {
      continue;
    }

    char c = client.read();
    Serial.write(c);

    if (c == ' ') {
      return;
    }

    action->concat(c);
  }
}

void ignoreRequest() {
  String currentLine = "";

  while (client.connected()) {
    if (!client.available()) {
      continue;
    }

    char c = client.read();
    Serial.write(c);

    if (c == '\n') {
      if (currentLine.length() == 0) {
        client.println("HTTP/1.1 200 OK");
        client.println("Content-type:text/html");
        client.println("Connection: close");
        client.println("");

        client.println("<link rel=\"stylesheet\" href=\"https://cdn.jsdelivr.net/npm/water.css@2/out/water.css\">");

        break;
      } else {
        currentLine = "";
      }
    } else if (c != '\r') {
      currentLine += c;
    }
  }
}

void handleAction(PerformAction performAction) {
  strAction = "";
  actionReadIndex = 0;

  readAction(&strAction);
  ignoreRequest();

  Serial.print("\n\nHTTP ACTION: ");
  Serial.println(strAction);

  client.print("<h1>ACTION</h1>");
  client.print("<p>");
  client.print(strAction);
  client.print("</p>");

  client.print("<h2>RESULT</h2>");

  client.print("<pre><code>");
  SerialAction action = intToAction(webReadInt());
  performAction(action, webPrintStr, webPrintInt, webReadInt);
  client.print("</code></pre>");

  client.print("<hr/>");
  client.print("<p><a href=\"/\">BACK</a></p>");
}


void printCommand(int code, char name[]) {
  client.print("<tr><td>");
  client.print(code);
  client.print("</td><td>");
  client.print(name);
  client.print("</td><td>");
}

void printCommandButton(int code, char name[], char args[]) {
  client.print("<a href=\"/action/");
  client.print(code);
  client.print("/");
  client.print(args);
  client.print("\">");

  client.print(name);
  client.print("</a> &nbsp;");
}

void doneCommand() {
  client.print("</td></tr>");
}

void genericAction(int code, char name[]) {
  printCommand(code, name);

  printCommandButton(code, "all", "");

  doneCommand();
}

void genericGroupAction(int code, char name[]) {
  printCommand(code, name);

  for (int i = 0; i < TOTAL_GROUPS; i++) {
    String actionName = String("group ") + i;
    printCommandButton(code, actionName.c_str(), String(i).c_str());
  }

  doneCommand();
}

void colorActionRow() {
  printCommand(5, "Set color");

  printCommandButton(5, "extra white", "255/255/255");
  printCommandButton(5, "white", "180/180/180");
  printCommandButton(5, "low white", "80/80/80");

  printCommandButton(5, "indigo", "75/0/130");
  printCommandButton(5, "purple", "216/191/216");
  printCommandButton(5, "orange", "255/165/0");
  printCommandButton(5, "yellow", "255/255/0");
  printCommandButton(5, "teal", "128/255/255");
  printCommandButton(5, "cyan", "0/255/255");


  printCommandButton(5, "red", "255/0/0");
  printCommandButton(5, "pink", "227/151/172");

  printCommandButton(5, "green", "0/255/0");
  printCommandButton(5, "blue", "0/0/255");

  doneCommand();
}

void audioThresholdRow() {
  printCommand(9, "Audio threshold");

  printCommandButton(9, "low", "60");
  printCommandButton(9, "normal", "140");
  printCommandButton(9, "high", "250");
  printCommandButton(9, "disabled", "500");

  doneCommand();
}

void printWEB(FlushReport report, PerformAction performAction) {
  if (!client) {
    return;
  }

  Serial.println("new client");  // print a message out the serial port
  String currentLine = "";       // make a String to hold incoming data from the client

  while (client.connected()) {  // loop while the client's connected
    if (!client.available()) {
      continue;
    }

    char c = client.read();  // read a byte, then

    Serial.write(c);  // print it out the serial monitor

    if (c == '\n') {
      // if the byte is a newline character
      // if the current line is blank, you got two newline characters in a row.
      // that's the end of the client HTTP request, so send a response:

      if (currentLine.length() == 0) {
        // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
        // and a content-type so the client knows what's coming, then a blank line:

        client.println("HTTP/1.1 200 OK");
        client.println("Content-type:text/html");
        client.println();

        client.println("<link rel=\"stylesheet\" href=\"https://cdn.jsdelivr.net/npm/water.css@2/out/water.css\">");

        client.print("<pre><code>");
        report(webPrintStr, webPrintInt);
        client.print("</code></pre><hr/>");

        //create the buttons
        client.print("<table>");
        client.print("<thead>");
        client.print("<tr><th>Code</th><th>Name</th><th>&nbsp;</th></tr>");
        client.print("</thead>");

        client.print("<tbody>");

        genericGroupAction(0, "Show group");
        genericGroupAction(1, "Happy");
        genericGroupAction(2, "Flicker");
        genericGroupAction(4, "Hide");
        colorActionRow();
        genericAction(6, "Report");
        genericAction(7, "All happy");
        genericAction(8, "Stamina");
        audioThresholdRow();
        genericAction(11, "Query audio");
        genericAction(12, "Measure distance");
        genericGroupAction(13, "Hilight flicker");
        genericAction(14, "Hide all");
        genericAction(15, "Raw color");

        client.print("</tbody>");

        client.print("</table>");

        // The HTTP response ends with another blank line:
        client.println();
        // break out of the while loop:
        break;
      } else {  // if you got a newline, then clear currentLine:
        currentLine = "";
      }
    } else if (c != '\r') {  // if you got anything else but a carriage return character,
      currentLine += c;      // add it to the end of the currentLine
    }

    if (currentLine.endsWith("GET /action/")) {
      handleAction(performAction);
      break;
    }
  }

  // close the connection:
  client.stop();
  Serial.println("client disconnected");
}