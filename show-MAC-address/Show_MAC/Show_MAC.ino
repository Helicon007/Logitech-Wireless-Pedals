#include <ESP8266WiFi.h>  // Bibliothek für WiFi und MAC-Funktionen

void setup() {
  Serial.begin(115200);
  delay(1000); // kurze Pause, damit der Serial Monitor bereit ist

  // Hole die MAC-Adresse im STA-Modus
  WiFi.mode(WIFI_STA);
  String mac = WiFi.macAddress();

  Serial.println("");
  Serial.println("MAC-Adresse: ");
  Serial.println(mac);
}

void loop() {
  // nichts zu tun
}
