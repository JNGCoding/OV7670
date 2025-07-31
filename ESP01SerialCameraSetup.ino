#include <Arduino.h>
#include <HardwareSerial.h>
#include <ESP8266WiFi.h>

// Classes (if Any)

// Variables
const char* CameraSSID = "CameraIP";
const char* CameraPass = "12345678";
WiFiServer Camera(80);
WiFiClient CameraClient;

// Functions
void exit() { while (1) yield(); }

void latch_client() {
  while (!Camera.hasClient()) yield();
  CameraClient = Camera.accept();

  digitalWrite(4, HIGH);
}

void setup() {
  Serial.begin(2000000);
  Serial.println();

  pinMode(5, OUTPUT);
  pinMode(4, OUTPUT);
  digitalWrite(5, LOW);
  digitalWrite(4, LOW);

  WiFi.softAP(CameraSSID, CameraPass);
  delay(50);

  Camera.begin();

  Serial.println();
  Serial.println(WiFi.softAPIP());

  digitalWrite(5, HIGH);
}

void loop() {
  latch_client();

  while (CameraClient.connected()) {
    if (Serial.available()) {
      while (Serial.available() > 0) CameraClient.write(Serial.read());
    }
    if (CameraClient.available()) {
      while (CameraClient.available() > 0) Serial.write(CameraClient.read());
    }
  }

  digitalWrite(4, LOW);
}
