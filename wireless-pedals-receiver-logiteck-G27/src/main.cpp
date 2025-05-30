#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPNowW.h>
#include <stdio.h>
#include <U8g2lib.h>

typedef struct struct_message {
  int16_t a0;
  int16_t a1;
  int16_t a2;
  int16_t a3;
} struct_message;

struct_message incomingData;

// ADS1115 Settings
#define ADS_RANGE 4.096  // GAIN_ONE
#define ADS_MAX_VALUE 32767.0  // 15bit
#define INPUT_VOLTAGE_MAX 3.3

// Display graphics
static const uint8_t image_battery_33_bits[] = {0x00,0x00,0x00,0xf0,0xff,0x7f,0x08,0x00,0x80,0x08,0x00,0xb6,0x0e,0x00,0xb6,0x01,0x00,0xb6,0x01,0x00,0xb6,0x01,0x00,0xb6,0x01,0x00,0xb6,0x01,0x00,0xb6,0x0e,0x00,0xb6,0x08,0x00,0xb6,0x08,0x00,0x80,0xf0,0xff,0x7f,0x00,0x00,0x00,0x00,0x00,0x00};
static const uint8_t image_battery_67_bits[] = {0x00,0x00,0x00,0xf0,0xff,0x7f,0x08,0x00,0x80,0x08,0xd8,0xb6,0x0e,0xd8,0xb6,0x01,0xd8,0xb6,0x01,0xd8,0xb6,0x01,0xd8,0xb6,0x01,0xd8,0xb6,0x01,0xd8,0xb6,0x0e,0xd8,0xb6,0x08,0xd8,0xb6,0x08,0x00,0x80,0xf0,0xff,0x7f,0x00,0x00,0x00,0x00,0x00,0x00};
static const uint8_t image_battery_83_bits[] = {0x00,0x00,0x00,0xf0,0xff,0x7f,0x08,0x00,0x80,0x08,0xdb,0xb6,0x0e,0xdb,0xb6,0x01,0xdb,0xb6,0x01,0xdb,0xb6,0x01,0xdb,0xb6,0x01,0xdb,0xb6,0x01,0xdb,0xb6,0x0e,0xdb,0xb6,0x08,0xdb,0xb6,0x08,0x00,0x80,0xf0,0xff,0x7f,0x00,0x00,0x00,0x00,0x00,0x00};
static const uint8_t image_battery_empty_bits[] = {0x00,0x00,0x00,0xf0,0xff,0x7f,0x08,0x00,0x80,0x08,0x00,0x80,0x0e,0x00,0x80,0x01,0x00,0x80,0x01,0x00,0x80,0x01,0x00,0x80,0x01,0x00,0x80,0x01,0x00,0x80,0x0e,0x00,0x80,0x08,0x00,0x80,0x08,0x00,0x80,0xf0,0xff,0x7f,0x00,0x00,0x00,0x00,0x00,0x00};
static const uint8_t image_battery_full_bits[] = {0x00,0x00,0x00,0xf0,0xff,0x7f,0x08,0x00,0x80,0x68,0xdb,0xb6,0x6e,0xdb,0xb6,0x61,0xdb,0xb6,0x61,0xdb,0xb6,0x61,0xdb,0xb6,0x61,0xdb,0xb6,0x61,0xdb,0xb6,0x6e,0xdb,0xb6,0x68,0xdb,0xb6,0x08,0x00,0x80,0xf0,0xff,0x7f,0x00,0x00,0x00,0x00,0x00,0x00};
static const uint8_t image_radio_bits[] = {0x00,0x00,0x00,0x00,0x00,0x00,0x04,0x40,0x00,0x02,0x80,0x00,0x12,0x90,0x00,0x09,0x21,0x01,0xa5,0x4b,0x01,0x95,0x52,0x01,0xa5,0x4b,0x01,0x09,0x21,0x01,0x12,0x90,0x00,0x02,0x80,0x00,0x04,0x40,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

// OLED
// U8G2_SH1106_128X32_VISIONOX_1_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
//U8G2_SSD1306_128X32_UNIVISION_1_SW_I2C u8g2(U8G2_R0, /* clock=*/ 21, /* data=*/ 20, /* reset=*/ U8X8_PIN_NONE);   // Adafruit Feather M0 Basic Proto + FeatherWing OLED
//U8G2_SSD1306_128X32_UNIVISION_1_SW_I2C u8g2(U8G2_R0, /* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);   // Adafruit Feather ESP8266/32u4 Boards + FeatherWing OLED
U8G2_SSD1306_128X32_UNIVISION_1_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);   // Adafruit ESP8266/32u4/ARM Boards + FeatherWing OLED
//U8G2_SSD1306_128X32_UNIVISION_1_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ SCL, /* data=*/ SDA);   // pin remapping with ESP8266 HW I2C
// U8G2_SSD1306_128X32_WINSTAR_1_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ SCL, /* data=*/ SDA);   // pin remapping with ESP8266 HW I2C


// PWM Pins (z. B. D1, D2, D3)
const int pwmPin1 = D5;  // accerleration
const int pwmPin2 = D6;  // brake
const int pwmPin3 = D7;  // clutch

char u_bat[5];
unsigned long previousTime = 0;
unsigned long actualTime = millis();


void OnDataRecv(uint8_t *mac, uint8_t *incomingDataRaw, uint8_t len);

void updateOLED(float voltage, int acc, int brake);

void setup() {
  Serial.begin(57600);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  pinMode(pwmPin1, OUTPUT);
  pinMode(pwmPin2, OUTPUT);
  pinMode(pwmPin3, OUTPUT);

  analogWriteRange(1023);  // 10-bit PWM

  if (esp_now_init() != 0) {
    Serial.println("Fehler beim Initialisieren von ESP-NOW");
    return;
  }

  // OLED Init
  delay(250); // wait for the OLED to power up
  u8g2.begin();

  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
  esp_now_register_recv_cb(OnDataRecv);

  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.setDrawColor(1);
    // no data so far
    u8g2.drawStr(12,13,"Device ready");
    u8g2.drawStr(12,27,"no data received");
    } while ( u8g2.nextPage() );
}

void OnDataRecv(uint8_t *mac, uint8_t *incomingDataRaw, uint8_t len) {
  memcpy(&incomingData, incomingDataRaw, sizeof(incomingData));

  // Umrechnung: Rohwert → Spannung → PWM (0–1023)
  float v0 = (incomingData.a0 * ADS_RANGE) / ADS_MAX_VALUE;
  float v1 = (incomingData.a1 * ADS_RANGE) / ADS_MAX_VALUE;
  float v2 = (incomingData.a2 * ADS_RANGE) / ADS_MAX_VALUE;
  float v3 = (incomingData.a3 * ADS_RANGE) / ADS_MAX_VALUE;

  // Begrenzen auf 0–3.3V
  v1 = constrain(v1, 0.0, INPUT_VOLTAGE_MAX);
  v2 = constrain(v2, 0.0, INPUT_VOLTAGE_MAX);
  v3 = constrain(v3, 0.0, INPUT_VOLTAGE_MAX);

  int pwm1 = (v1 / INPUT_VOLTAGE_MAX) * 1023;
  int pwm2 = (v2 / INPUT_VOLTAGE_MAX) * 1023;
  int pwm3 = (v3 / INPUT_VOLTAGE_MAX) * 1023;

  analogWrite(pwmPin1, pwm1);
  analogWrite(pwmPin2, pwm2);
  analogWrite(pwmPin3, pwm3);

  v0 = (v0 * 320) / 220;         // voltage divider  GND----220k--*--100k----V_bat 

  /* only for debug  
  dtostrf(v0, 4, 2, u_bat);
  Serial.printf("PWM1=%d raw=%d | PWM2=%d raw=%d | PWM3=%d raw=%d | Vbat=%sV raw=%d \n", pwm1, incomingData.a1, pwm2, incomingData.a2, pwm3, incomingData.a3, u_bat, incomingData.a0);
 */

  // OLED update once per second
  actualTime = millis();
  if (actualTime - previousTime >= 1000) {
    previousTime = actualTime;
    updateOLED(v0, pwm1, pwm2);
  }
  
}

void updateOLED(float voltage, int acc, int brake) {
  dtostrf(voltage, 4, 2, u_bat);
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.setDrawColor(1);
    // about the battery
    u8g2.drawStr(12,13,u_bat);
    u8g2.drawStr(36,13,"V");
    if (voltage >= 3.9) {
      u8g2.drawXBM(56, 2, 24, 16, image_battery_full_bits);
    } else if (voltage >= 3.7) {
      u8g2.drawXBM(56, 2, 24, 16, image_battery_83_bits);
    } else if (voltage >= 3.5) {
      u8g2.drawXBM(56, 2, 24, 16, image_battery_67_bits);     
    } else if (voltage >= 3.2) {
      u8g2.drawXBM(56, 2, 24, 16, image_battery_33_bits);     
    }  else {
      u8g2.drawXBM(56, 2, 24, 16, image_battery_empty_bits);     
    }

    u8g2.drawXBM(99, 2, 17, 16, image_radio_bits);  // makes actually no sense because only with radio the display is updated, but it looks nice ;o) 

    // about the acc and brake pedal
    acc = constrain(acc, 150, 940);
    brake = constrain(brake, 150, 940);
    int a = map(acc, 940, 150, 0, 100);
    int b = map(brake, 940, 150, 0, 100);
    u8g2.drawBox(14, 20, a, 4);
    u8g2.drawBox(14, 28, b, 4);

    
  } while ( u8g2.nextPage() );
}

void loop() {
  // do nothing
}
