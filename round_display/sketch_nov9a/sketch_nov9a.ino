#include <Adafruit_GFX.h>
#include <Adafruit_GC9A01A.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>

#define TFT_CS   D8
#define TFT_DC   D3
#define TFT_RST  D4

Adafruit_GC9A01A tft(TFT_CS, TFT_DC, TFT_RST);

// WiFi credentials
const char* ssid = "AlirezaHome_24";
const char* password = "Ali1351Reza";

// OpenWeatherMap setup
const String apiKey = "364a1c2935392f2fd5fe1b2a91845fab";
const String city = "Yokohama";
const String country = "JP";

const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 9 * 3600;   // Japan time (UTC+9)
const int daylightOffset_sec = 0;

int centerX = 120;
int centerY = 120;
int clockRadius = 110;

// To store weather info
String weatherDesc = "";
float temperature = 0.0;

// Time tracking
unsigned long lastWeatherUpdate = 0;
int prevSec = -1;

// Function declarations
void drawClockFace();
void drawHands(struct tm timeinfo);
void fetchWeather();
void showWeather();

void setup() {
  Serial.begin(115200);

  tft.setSPISpeed(40000000);
  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(GC9A01A_BLACK);
  tft.setTextColor(GC9A01A_GREEN);
  tft.setTextSize(2);
  tft.setCursor(40, 110);
  tft.println("Starting...");
  delay(1000);

  // Draw static clock face
  drawClockFace();

  // Connect to WiFi
  Serial.printf("Connecting to WiFi %s...\n", ssid);
  tft.setCursor(20, 220);
  tft.setTextSize(1);
  tft.setTextColor(GC9A01A_CYAN);
  tft.println("Connecting WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
  tft.fillRect(0, 210, 240, 30, GC9A01A_BLACK);
  tft.setCursor(20, 220);
  tft.println("WiFi connected");

  // Setup NTP time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  // Fetch first weather update
  fetchWeather();
  showWeather();
}

void loop() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    delay(1000);
    return;
  }

  int currentSec = timeinfo.tm_sec;
  if (currentSec != prevSec) {
    drawHands(timeinfo);
    prevSec = currentSec;
  }

  // Update weather every 15 minutes
  if (millis() - lastWeatherUpdate > 900000) {
    fetchWeather();
    showWeather();
    lastWeatherUpdate = millis();
  }
}

void drawClockFace() {
  tft.fillScreen(GC9A01A_BLACK);
  tft.drawCircle(centerX, centerY, clockRadius, GC9A01A_WHITE);

  for (int i = 0; i < 12; i++) {
    float angle = i * 30 * DEG_TO_RAD;
    int x1 = centerX + sin(angle) * (clockRadius - 10);
    int y1 = centerY - cos(angle) * (clockRadius - 10);
    int x2 = centerX + sin(angle) * clockRadius;
    int y2 = centerY - cos(angle) * clockRadius;
    tft.drawLine(x1, y1, x2, y2, GC9A01A_WHITE);
  }

  tft.setCursor(60, 210);
  tft.setTextSize(1);
  tft.print("Yokohama Clock");
}

void drawHands(struct tm timeinfo) {
  static int oldSecX, oldSecY;
  static int oldMinX, oldMinY;
  static int oldHourX, oldHourY;
  static bool firstDraw = true;

  // Calculate angles
  float secAngle  = radians(timeinfo.tm_sec * 6);
  float minAngle  = radians(timeinfo.tm_min * 6 + timeinfo.tm_sec * 0.1);
  float hourAngle = radians((timeinfo.tm_hour % 12) * 30 + timeinfo.tm_min * 0.5);

  // Calculate new positions
  int secX  = centerX + sin(secAngle) * (clockRadius - 15);
  int secY  = centerY - cos(secAngle) * (clockRadius - 15);
  int minX  = centerX + sin(minAngle) * (clockRadius - 25);
  int minY  = centerY - cos(minAngle) * (clockRadius - 25);
  int hourX = centerX + sin(hourAngle) * (clockRadius - 45);
  int hourY = centerY - cos(hourAngle) * (clockRadius - 45);

  // Erase old hands
  if (!firstDraw) {
    tft.drawLine(centerX, centerY, oldHourX, oldHourY, GC9A01A_BLACK);
    tft.drawLine(centerX, centerY, oldMinX, oldMinY, GC9A01A_BLACK);
    tft.drawLine(centerX, centerY, oldSecX, oldSecY, GC9A01A_BLACK);
  }

  // Draw new hands
  tft.drawLine(centerX, centerY, hourX, hourY, GC9A01A_YELLOW);
  tft.drawLine(centerX, centerY, minX, minY, GC9A01A_CYAN);
  tft.drawLine(centerX, centerY, secX, secY, GC9A01A_RED);

  // Save positions
  oldSecX = secX; oldSecY = secY;
  oldMinX = minX; oldMinY = minY;
  oldHourX = hourX; oldHourY = hourY;

  firstDraw = false;
}

void fetchWeather() {
  if (WiFi.status() != WL_CONNECTED) return;

  WiFiClient client;
  HTTPClient http;

  String url = "http://api.openweathermap.org/data/2.5/weather?q=" + city + "," + country +
               "&appid=" + apiKey + "&units=metric";
  Serial.println(url);
  if (!http.begin(client, url)) {
    Serial.println("HTTP begin failed!");
    return;
  }

  int httpCode = http.GET();
  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    DynamicJsonDocument doc(2048);
    deserializeJson(doc, payload);

    temperature = doc["main"]["temp"].as<float>();
    weatherDesc = doc["weather"][0]["main"].as<String>();
    Serial.printf("Weather: %s %.1fC\n", weatherDesc.c_str(), temperature);
  } else {
    Serial.printf("Weather fetch failed, code %d\n", httpCode);
  }

  http.end();
}

void showWeather() {
  tft.fillRect(0, 0, 240, 20, GC9A01A_BLACK);
  tft.setCursor(20, 5);
  tft.setTextSize(1);
  tft.setTextColor(GC9A01A_GREEN);
  tft.printf("%s  %.1fC", weatherDesc.c_str(), temperature);
}