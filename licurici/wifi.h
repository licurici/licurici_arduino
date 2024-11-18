#include <WiFiNINA.h>
#include "types.h"

int enable_WiFi();
int connect_WiFi();
void printWifiStatus();
void printWEB(FlushReport, PerformAction);
int setup_WiFi();
void loop_WiFi(FlushReport, PerformAction);