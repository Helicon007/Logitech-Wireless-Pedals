#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_ADS1X15.h>
#include <ESP8266WiFi.h>
#include <ESPNowW.h>

#define CHANNEL 1  // ESP-NOW Channel
#define SAMPLE_RATE_HZ 120
#define INTERVAL_MS (1000 / SAMPLE_RATE_HZ)
//ESP-01 i2c pins (arduino defaults to 4/5)
#define SDA 0
#define SCL 2

Adafruit_ADS1115 ads;

uint8_t peerAddress[] = {0xec, 0xfa, 0xbc, 0x29, 0xb9, 0x8e}; // MAC-Adresse des Empfängers

// Struktur für die zu sendenden Daten
typedef struct struct_message {
  int16_t a0;
  int16_t a1;  // accerleration
  int16_t a2;  // brake
  int16_t a3;  // clutch
} struct_message;

struct_message data;

unsigned long lastSendTime = 0;

void setup() {
  Serial.begin(115200);
  Wire.begin(SDA, SCL);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  if (!ads.begin()) {
    Serial.println("ADS1115 nicht gefunden!");
    while (1);
  }

  ads.setDataRate(RATE_ADS1115_860SPS);  // max Sample-Rate
  ads.setGain(GAIN_ONE);

  if (esp_now_init() != 0) {
    Serial.println("Fehler beim Initialisieren von ESP-NOW");
    return;
  }

  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  esp_now_add_peer(peerAddress, ESP_NOW_ROLE_SLAVE, CHANNEL, NULL, 0);
}

void loop() {
  unsigned long now = millis();
  if (now - lastSendTime >= INTERVAL_MS) {
    lastSendTime = now;

    data.a0 = ads.readADC_SingleEnded(0);
    data.a1 = ads.readADC_SingleEnded(1);
    data.a2 = ads.readADC_SingleEnded(2);
    data.a3 = ads.readADC_SingleEnded(3);

    esp_now_send(peerAddress, (uint8_t *)&data, sizeof(data));
    delay(4);  // the ads1115 can't go faster anyway, so why not rest for a moment to save 20% energy?
  }
}
