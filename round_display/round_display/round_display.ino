#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <ESP8266HTTPClient.h>
#include <Adafruit_GFX.h>
#include <Adafruit_GC9A01A.h>
#include <SPI.h>

// ----------------- DISPLAY SETUP -----------------
#define TFT_CS   D8
#define TFT_DC   D1
#define TFT_RST  D4
Adafruit_GC9A01A tft(TFT_CS, TFT_DC, TFT_RST);

// ----------------- WIFI & NTP -----------------
const char* ssid = "AlirezaHome_24";
const char* password = "Ali1351Reza";
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 9 * 3600, 60000); // JST (UTC+9)

// ----------------- WEATHER -----------------
const char* weatherUrl = "http://api.openweathermap.org/data/2.5/weather?q=Yokohama,jp&appid=364a1c2935392f2fd5fe1b2a91845fab&units=metric";
float currentTemp = 0.0;
String weatherDesc = "Loading...";
unsigned long lastWeatherUpdate = 0;
const unsigned long weatherInterval = 600000; // 10 min

// ----------------- CLOCK CENTER -----------------
#define CX 120
#define CY 120
#define RADIUS 115

// ----------------- COLORS -----------------
#define COLOR_HOUR   GC9A01A_WHITE
#define COLOR_MINUTE GC9A01A_WHITE
#define COLOR_SECOND GC9A01A_RED

// ----------------- INIT -----------------
void setup() {
  Serial.begin(115200);
  delay(500);

  SPI.begin();
  delay(200);
  tft.begin();
  tft.setRotation(1);  // Landscape mode
  tft.fillScreen(GC9A01A_BLACK);
  tft.setTextColor(GC9A01A_GREEN);
  tft.setTextSize(2);
  tft.setCursor(20, 100);
  tft.println("Display Init OK");
  delay(1000);

  WiFi.begin(ssid, password);
  tft.fillScreen(GC9A01A_BLACK);
  tft.setCursor(20, 100);
  tft.println("Connecting WiFi...");
  Serial.print("Connecting WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected!");
  tft.fillScreen(GC9A01A_BLACK);
  tft.setCursor(20, 100);
  tft.println("WiFi Connected!");

  timeClient.begin();
  timeClient.update();
  fetchWeather();
  delay(1500);
  tft.fillScreen(GC9A01A_BLACK);
}

// ----------------- WEATHER FUNCTION -----------------
void fetchWeather() {
  if (WiFi.status() != WL_CONNECTED) return;

  WiFiClient client;
  HTTPClient http;

  Serial.println("Fetching weather...");
  if (http.begin(client, weatherUrl)) {
    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      Serial.println(payload);

      int tempIndex = payload.indexOf("\"temp\":");
      if (tempIndex > 0) {
        currentTemp = payload.substring(tempIndex + 7, payload.indexOf(",", tempIndex)).toFloat();
      }

      // extract "main":"Clear" or similar
      int mainIndex = payload.indexOf("\"main\":\"");
      if (mainIndex > 0) {
        int end = payload.indexOf("\"", mainIndex + 8);
        weatherDesc = payload.substring(mainIndex + 8, end);
      }
    } else {
      Serial.printf("HTTP error %d\n", httpCode);
    }
    http.end();
  } else {
    Serial.println("HTTP begin failed!");
  }
  lastWeatherUpdate = millis();
}

// ----------------- DRAWING -----------------
void drawClockFace() {
  tft.drawCircle(CX, CY, RADIUS, GC9A01A_DARKGREY);
  for (int i = 0; i < 60; i++) {
    float angle = i * 6 * DEG_TO_RAD;
    int x1 = CX + cos(angle) * (RADIUS - 10);
    int y1 = CY + sin(angle) * (RADIUS - 10);
    int x2 = CX + cos(angle) * (RADIUS - ((i % 5 == 0) ? 20 : 15));
    int y2 = CY + sin(angle) * (RADIUS - ((i % 5 == 0) ? 20 : 15));
    tft.drawLine(x1, y1, x2, y2, GC9A01A_DARKGREY);
  }
}

void drawHand(float angle, int length, int color, int thickness) {
  int x = CX + cos(angle) * length;
  int y = CY + sin(angle) * length;
  tft.drawLine(CX, CY, x, y, color);
  for (int i = 1; i < thickness; i++) {
    tft.drawLine(CX + i, CY, x + i, y, color);
  }
}

void drawWeather() {
  tft.fillRect(0, 0, 240, 40, GC9A01A_BLACK);
  tft.setTextColor(GC9A01A_YELLOW);
  tft.setTextSize(2);
  tft.setCursor(10, 10);
  tft.printf("%s %.1fC", weatherDesc.c_str(), currentTemp);
}

void drawDate() {
  time_t rawTime = timeClient.getEpochTime();
  struct tm *timeInfo = localtime(&rawTime);

  char dateStr[40];
  strftime(dateStr, sizeof(dateStr), "%a, %b %d %Y", timeInfo);

  tft.fillRect(0, 200, 240, 40, GC9A01A_BLACK);
  tft.setTextColor(GC9A01A_CYAN);
  tft.setTextSize(2);
  int16_t x1, y1;
  uint16_t w, h;
  tft.getTextBounds(dateStr, 0, 0, &x1, &y1, &w, &h);
  tft.setCursor((240 - w) / 2, 220);
  tft.print(dateStr);
}

// ----------------- MAIN LOOP -----------------
void loop() {
  timeClient.update();
  time_t rawTime = timeClient.getEpochTime();
  struct tm *timeInfo = localtime(&rawTime);

  if (millis() - lastWeatherUpdate > weatherInterval) {
    fetchWeather();
    drawWeather();
  }

  drawClockFace();
  //drawWeather();
  //drawDate();

  int sec = timeInfo->tm_sec;
  int min = timeInfo->tm_min;
  int hr  = timeInfo->tm_hour % 12;

  float secAngle = (sec * 6 - 90) * DEG_TO_RAD;
  float minAngle = (min * 6 - 90) * DEG_TO_RAD;
  float hrAngle  = ((hr * 30) + (min / 2) - 90) * DEG_TO_RAD;

  drawHand(hrAngle, 60, COLOR_HOUR, 3);
  drawHand(minAngle, 85, COLOR_MINUTE, 2);
  drawHand(secAngle, 100, COLOR_SECOND, 1);

  delay(1000);
  // erase second hand for next frame
  tft.drawLine(CX, CY, CX + cos(secAngle) * 100, CY + sin(secAngle) * 100, GC9A01A_BLACK);
}
