// for ESP only

#include <WiFi.h>

const char* ssid = "YOUR_SSID";          // replace with SSID of your WiFi network
const char* password = "YOUR_PASSWORD";  // replace with password of your WiFi network

void setup() {
  Serial.begin(115200);
  delay(2000);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println();
  Serial.println();

  Serial.println("Use following values for the \"staticIP\" example");
  Serial.println("Note: replace \".\" with \",\"");
  Serial.println();

  Serial.print("Local IP: ");
  Serial.print('\t');
  Serial.print("(");
  Serial.print(WiFi.localIP());
  Serial.print(")");
  Serial.println(" (this one not needed)");

  Serial.print("Subnet mask: ");
  Serial.print('\t');
  Serial.print("(");
  Serial.print(WiFi.subnetMask());
  Serial.println(")");

  Serial.print("Gateway: ");
  Serial.print('\t');
  Serial.print("(");
  Serial.print(WiFi.gatewayIP());
  Serial.println(")");

  Serial.print("DNS 1:");
  Serial.print('\t');
  Serial.print('\t');
  Serial.print("(");
  Serial.print(WiFi.dnsIP(0));
  Serial.println(")");

  Serial.print("DNS 2: ");
  Serial.print('\t');
  Serial.print('\t');
  Serial.print("(");
  Serial.print(WiFi.dnsIP(1));
  Serial.println(")");
}

void loop() {
  // do nothing
}