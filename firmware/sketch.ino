#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include "aq_model.h"

// ---- WiFi / ThingSpeak settings ----
const char* WIFI_SSID    = "Wokwi-GUEST";       
const char* WIFI_PASS    = "";
const char* TS_WRITE_KEY = "YOUR_WRITE_API_KEY";
const unsigned long UPLOAD_INTERVAL_MS = 20000;  

// ---- Pins (ADC1 pins, safe to use with WiFi) ----
const int PIN_CO  = 1;
const int PIN_NO2 = 2;
const int PIN_SO2 = 4;
const int PIN_O3  = 5;

const int LED_GREEN  = 15;
const int LED_YELLOW = 16;
const int LED_RED    = 17;
const int BUZZER     = 18;

// ---- Full-scale value of each pot, in the SAME units as the training CSV ----
const float CO_MAX  = 3534.0f;
const float NO2_MAX = 110.0f;
const float SO2_MAX = 173.0f;
const float O3_MAX  = 212.0f;

const int ADC_MAX = 4095;   // 12-bit ADC

const char* CLASS_NAMES[] = {"Good", "Moderate", "Poor"};
unsigned long lastUpload = 0;

// ---- Simulated sensor noise (simulation only) ----
const bool  SIMULATE_NOISE = true;
const float NOISE_FRACTION = 0.01f;   

float readScaled(int pin, float maxValue) {
  float v = (analogRead(pin) / (float)ADC_MAX) * maxValue;
  if (SIMULATE_NOISE) {
    v += (random(-1000, 1001) / 1000.0f) * NOISE_FRACTION * maxValue;
    if (v < 0) v = 0;
    if (v > maxValue) v = maxValue;
  }
  return v;
}

void showClass(int cls) {
  digitalWrite(LED_GREEN,  cls == 0);
  digitalWrite(LED_YELLOW, cls == 1);
  digitalWrite(LED_RED,    cls == 2);
  if (cls == 2) tone(BUZZER, 1000);   // alarm only for Poor
  else          noTone(BUZZER);
}

bool connectWiFi() {
  if (WiFi.status() == WL_CONNECTED) return true;
  Serial.print("Connecting to WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASS, 6);          // channel 6 speeds up connection in Wokwi
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) {
    delay(250);
    Serial.print(".");
  }
  Serial.println(WiFi.status() == WL_CONNECTED ? " connected" : " FAILED");
  return WiFi.status() == WL_CONNECTED;
}

bool uploadToThingSpeak(float co, float no2, float so2, float o3, int cls) {
  if (!connectWiFi()) return false;

  WiFiClientSecure client;
  client.setInsecure();  
  HTTPClient http;

  // NOTE: Arduino's String(float, digits) pads small numbers with a leading space
  char params[128];
  snprintf(params, sizeof(params),
           "&field1=%.0f&field2=%.1f&field3=%.1f&field4=%.1f&field5=%d",
           co, no2, so2, o3, cls);
  String url = String("https://api.thingspeak.com/update?api_key=") + TS_WRITE_KEY + params;

  if (!http.begin(client, url)) {
    Serial.println("HTTP begin failed");
    return false;
  }
  int code = http.GET();
  String body = http.getString(); 
  http.end();

  Serial.printf("ThingSpeak: HTTP %d, entry id %s\n", code, body.c_str());
  return code == 200 && body != "0";
}

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  connectWiFi();
}

void loop() {
  float co  = readScaled(PIN_CO,  CO_MAX);
  float no2 = readScaled(PIN_NO2, NO2_MAX);
  float so2 = readScaled(PIN_SO2, SO2_MAX);
  float o3  = readScaled(PIN_O3,  O3_MAX);

  // On-device inference
  int cls = aq_predict(co, no2, so2, o3);
  showClass(cls);

  // Inference time: average over many calls with inputs the compiler cannot pre-compute
  volatile float v_co = co, v_no2 = no2, v_so2 = so2, v_o3 = o3;
  volatile int sink = 0;
  unsigned long t0 = micros();
  for (int i = 0; i < 1000; i++) {
    sink = aq_predict(v_co, v_no2, v_so2, v_o3);
  }
  float usPerCall = (micros() - t0) / 1000.0f;

  Serial.printf("CO=%.0f NO2=%.1f SO2=%.1f O3=%.1f -> %s (%.2f us/inference)\n",
                co, no2, so2, o3, CLASS_NAMES[cls], usPerCall);

  // Periodic cloud upload (first upload happens immediately)
  if (lastUpload == 0 || millis() - lastUpload >= UPLOAD_INTERVAL_MS) {
    uploadToThingSpeak(co, no2, so2, o3, cls);
    lastUpload = millis();
  }

  delay(1000);
}
