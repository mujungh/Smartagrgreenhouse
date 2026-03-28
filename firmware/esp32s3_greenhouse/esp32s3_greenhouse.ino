#include <WiFi.h>
#include <HTTPClient.h>
#include <ESP32Servo.h>
#include <math.h>

const char* WIFI_SSID = "YOUR_WIFI_NAME";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
const char* SERVER_URL = "http://YOUR_SERVER_IP:3000/api/state";
const char* API_KEY = "smartagr-demo-key";
const char* CAMERA_STREAM_URL = "http://YOUR_ESP32_CAM_IP:81/stream";

// Update the pin mapping to match your ESP32-S3 Dev Module wiring.
const int LDR_PIN = 4;
const int THERMISTOR_PIN = 5;
const int FAN_PIN = 6;
const int SERVO_PIN = 7;

const float SERIES_RESISTOR = 10000.0f;
const float NOMINAL_RESISTANCE = 10000.0f;
const float NOMINAL_TEMPERATURE = 25.0f;
const float BETA_COEFFICIENT = 3950.0f;
const float ADC_MAX = 4095.0f;

const float FAN_ON_TEMP = 30.0f;
const float FAN_OFF_TEMP = 28.0f;
const int SHADE_CLOSE_BRIGHTNESS = 2500;
const int SHADE_OPEN_BRIGHTNESS = 1800;

const int SERVO_OPEN_ANGLE = 20;
const int SERVO_CLOSE_ANGLE = 100;
const unsigned long REPORT_INTERVAL_MS = 3000;

bool fanOn = false;
bool shadeClosed = false;
Servo shadeServo;

float readTemperatureC() {
  int adc = analogRead(THERMISTOR_PIN);
  if (adc <= 0 || adc >= ADC_MAX) {
    return -100.0f;
  }

  float resistance = SERIES_RESISTOR / ((ADC_MAX / (float)adc) - 1.0f);
  float steinhart = resistance / NOMINAL_RESISTANCE;
  steinhart = log(steinhart);
  steinhart /= BETA_COEFFICIENT;
  steinhart += 1.0f / (NOMINAL_TEMPERATURE + 273.15f);
  steinhart = 1.0f / steinhart;
  return steinhart - 273.15f;
}

int readBrightness() {
  return analogRead(LDR_PIN);
}

void connectWifi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Connecting to WiFi...");
    delay(500);
  }

  Serial.println("WiFi connected.");
  Serial.print("Local IP: ");
  Serial.println(WiFi.localIP());
}

void applyFanControl(float temperature) {
  if (!fanOn && temperature >= FAN_ON_TEMP) {
    fanOn = true;
  } else if (fanOn && temperature <= FAN_OFF_TEMP) {
    fanOn = false;
  }

  digitalWrite(FAN_PIN, fanOn ? HIGH : LOW);
}

void applyShadeControl(int brightness) {
  if (!shadeClosed && brightness >= SHADE_CLOSE_BRIGHTNESS) {
    shadeClosed = true;
  } else if (shadeClosed && brightness <= SHADE_OPEN_BRIGHTNESS) {
    shadeClosed = false;
  }

  shadeServo.write(shadeClosed ? SERVO_CLOSE_ANGLE : SERVO_OPEN_ANGLE);
}

void reportState(float temperature, int brightness) {
  if (WiFi.status() != WL_CONNECTED) {
    connectWifi();
  }

  HTTPClient http;
  http.begin(SERVER_URL);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", String("Bearer ") + API_KEY);

  String payload = "{";
  payload += "\"temperature\":" + String(temperature, 1) + ",";
  payload += "\"brightness\":" + String(brightness) + ",";
  payload += "\"fanOn\":" + String(fanOn ? "true" : "false") + ",";
  payload += "\"shadeClosed\":" + String(shadeClosed ? "true" : "false") + ",";
  payload += "\"cameraUrl\":\"" + String(CAMERA_STREAM_URL) + "\"";
  payload += "}";

  int httpCode = http.POST(payload);
  Serial.print("Report status code: ");
  Serial.println(httpCode);
  http.end();
}

void setup() {
  Serial.begin(115200);

  pinMode(FAN_PIN, OUTPUT);
  digitalWrite(FAN_PIN, LOW);

  analogReadResolution(12);
  shadeServo.attach(SERVO_PIN);
  shadeServo.write(SERVO_OPEN_ANGLE);

  connectWifi();
}

void loop() {
  float temperature = readTemperatureC();
  int brightness = readBrightness();

  applyFanControl(temperature);
  applyShadeControl(brightness);
  reportState(temperature, brightness);

  Serial.print("Temperature: ");
  Serial.print(temperature, 1);
  Serial.print(" C, Brightness: ");
  Serial.print(brightness);
  Serial.print(", Fan: ");
  Serial.print(fanOn ? "ON" : "OFF");
  Serial.print(", Shade: ");
  Serial.println(shadeClosed ? "CLOSED" : "OPEN");

  delay(REPORT_INTERVAL_MS);
}
