/*
  ESP8266 + GC9A01A (240x240 round) 
  Analog clock + Weather (Yokohama) dashboard
  Rotation = 1 (12 o'clock to the right)
  Partial redraw to avoid flicker.

  Libraries required:
  - Adafruit GFX
  - Adafruit GC9A01A
  - NTPClient
  - ArduinoJson
*/

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_GC9A01A.h>

// ---------------------- USER CONFIG ----------------------
const char* WIFI_SSID = "AlirezaHome_24";
const char* WIFI_PASS = "YOUR_WIFI_PASSWORD";

const char* OWM_API_KEY = "364a1c2935392f2fd5fe1b2a91845fab";
const char* CITY_NAME   = "Yokohama,JP"; // use city,country for reliability

const unsigned long WEATHER_INTERVAL_MS = 10UL * 60UL * 1000UL; // 10 minutes
const long GMT_OFFSET_SECONDS = 9L * 3600L; // JST
// ---------------------------------------------------------

// TFT pin mapping (confirm matches your wiring)
#define TFT_CS   D8  // Chip Select
#define TFT_DC   D1  // Data/Command
#define TFT_RST  D4  // Reset

Adafruit_GC9A01A tft(TFT_CS, TFT_DC, TFT_RST);

// Colors (RGB565)
#define COL_BLACK   0x0000
#define COL_WHITE   0xFFFF
#define COL_RED     0xF800
#define COL_YELLOW  0xFFE0
#define COL_CYAN    0x07FF
#define COL_LIGHTGR 0xC618
#define COL_DARKGR  0x8410

// Geometry
const int16_t TFT_W = 240;
const int16_t TFT_H = 240;
const int16_t CX = TFT_W / 2;
const int16_t CY = TFT_H / 2;
const int16_t RADIUS = 100;   // outer rim radius (keeps margin inside 240x240)

// NTP client
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", GMT_OFFSET_SECONDS, 60000);

// Weather state
float curTemp = NAN;
int curWeatherId = 800;
String curWeatherDesc = "";
unsigned long lastWeatherMs = 0;

// Keep last second so we update hands only when needed
int lastSecond = -1;

// Forward declarations
void fetchWeather() {
  if (WiFi.status() != WL_CONNECTED) return;

  WiFiClient client;        // for transport
  HTTPClient http;          // for HTTP logic

  String url = "http://api.openweathermap.org/data/2.5/weather?q=Yokohama,jp&appid=364a1c2935392f2fd5fe1b2a91845fab&units=metric";
  Serial.println("Fetching weather: " + url);

  if (http.begin(client, url)) {
    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      Serial.println("Weather payload:");
      Serial.println(payload);

      // Simple temperature extraction
      int tempIndex = payload.indexOf("\"temp\":");
      if (tempIndex > 0) {
        float temp = payload.substring(tempIndex + 7, payload.indexOf(",", tempIndex)).toFloat();
        Serial.print("Temp: ");
        Serial.println(temp);

        tft.setTextColor(GC9A01A_WHITE, GC9A01A_BLACK);
        tft.setTextSize(2);
        tft.setCursor(40, 20);
        tft.printf("☀ %.1f°C", temp);
      }
    } else {
      Serial.printf("HTTP Error: %d\n", httpCode);
    }
    http.end();
  } else {
    Serial.println("HTTP begin() failed!");
  }
}


  // Start NTP
  timeClient.begin();
  timeClient.update();

  // Initial weather fetch (if WiFi available)
  if (WiFi.status() == WL_CONNECTED) {
    fetchWeather();
    lastWeatherMs = millis();
  }

  // Draw static elements: rim, ticks, center dot, weather widget, bottom date area
  drawStaticFace();
  drawWeatherWidget();
  drawDate();
}

void loop() {
  // Update time from NTP client
  timeClient.update();

  // Refresh weather periodically
  if (WiFi.status() == WL_CONNECTED) {
    if (millis() - lastWeatherMs > WEATHER_INTERVAL_MS || lastWeatherMs == 0) {
      fetchWeather();
      lastWeatherMs = millis();
      // Update weather widget area
      drawWeatherWidget();
    }
  }

  int hh = timeClient.getHours();
  int mm = timeClient.getMinutes();
  int ss = timeClient.getSeconds();

  if (ss != lastSecond) {
    // Clear only the inner area of the clock (so rim/ticks remain)
    drawInnerClear();

    // Draw hands
    drawHands(hh, mm, ss);

    // draw date/other dynamic small text (only the parts that change)
    drawDate(); // date seldom changes but safe to call; it's small region

    lastSecond = ss;
  }

  delay(10); // small yield
}

// -------------------- Drawing / UI functions --------------------

void drawStaticFace() {
  // Clear full screen and draw outer rim & ticks
  tft.fillScreen(COL_BLACK);

  // outer rim
  tft.drawCircle(CX, CY, RADIUS, COL_WHITE);
  tft.drawCircle(CX, CY, RADIUS - 1, COL_DARKGR);

  // hour ticks (on rim)
  for (int i = 0; i < 12; ++i) {
    float ang = radians(i * 30.0 - 90.0);
    int16_t xOut = CX + cos(ang) * RADIUS;
    int16_t yOut = CY + sin(ang) * RADIUS;
    int16_t xIn = CX + cos(ang) * (RADIUS - (i % 3 == 0 ? 16 : 10));
    int16_t yIn = CY + sin(ang) * (RADIUS - (i % 3 == 0 ? 16 : 10));
    tft.drawLine(xIn, yIn, xOut, yOut, COL_WHITE);
  }

  // small center dot background
  tft.fillCircle(CX, CY, 4, COL_WHITE);

  // Pre-draw a black inner disc to define hands area (keeps rim & ticks visible)
  tft.fillCircle(CX, CY, RADIUS - 12, COL_BLACK);
}

void drawInnerClear() {
  // Clear the inner disc (where hands appear) WITHOUT touching the rim/ticks.
  // Use radius-12 to keep ticks intact.
  tft.fillCircle(CX, CY, RADIUS - 12, COL_BLACK);
  // redraw center dot after clear
  tft.fillCircle(CX, CY, 4, COL_WHITE);
}

void drawHands(int h, int m, int s) {
  // Angles in degrees (0 = 12 o'clock)
  float secondAng = s * 6.0;
  float minuteAng = m * 6.0 + s * 0.1;
  float hourAng = (h % 12) * 30.0 + m * 0.5;

  // Compute endpoints
  auto endPoint = [](int cx, int cy, float angDeg, int len) {
    float r = radians(angDeg - 90.0);
    int16_t x = cx + cos(r) * len;
    int16_t y = cy + sin(r) * len;
    return std::pair<int16_t, int16_t>(x, y);
  };

  // Hour hand (shorter, thicker, white)
  auto he = endPoint(CX, CY, hourAng, RADIUS - 52);
  tft.drawLine(CX, CY, he.first, he.second, COL_WHITE);
  // Optionally draw thicker hour by drawing adjacent lines
  tft.drawLine(CX+1, CY, he.first+1, he.second, COL_WHITE);
  tft.drawLine(CX-1, CY, he.first-1, he.second, COL_WHITE);

  // Minute hand (longer, white)
  auto me = endPoint(CX, CY, minuteAng, RADIUS - 28);
  tft.drawLine(CX, CY, me.first, me.second, COL_WHITE);

  // Second hand (thin, red)
  auto se = endPoint(CX, CY, secondAng, RADIUS - 16);
  tft.drawLine(CX, CY, se.first, se.second, COL_RED);

  // center pivot
  tft.fillCircle(CX, CY, 3, COL_WHITE);
}

void drawWeatherWidget() {
  // Position weather inside top area (inside circle, not touching rim)
  int16_t iconSize = 36;
  int16_t iconX = CX - iconSize/2;
  int16_t iconY = CY - RADIUS + 14; // 14 px from top of inner rim

  // Clear widget area
  tft.fillRect(iconX - 8, iconY - 6, iconSize + 120, iconSize + 30, COL_BLACK);

  // Draw simple icon (sun/cloud/rain) based on weather id
  int grp = curWeatherId / 100;
  if (curWeatherId == 800) grp = 80; // clear

  // icon center
  int cx = iconX + iconSize/2;
  int cy = iconY + iconSize/2;

  if (grp == 8 || grp == 80) {
    // Sun: small filled circle + rays
    tft.fillCircle(cx, cy, iconSize/4, COL_YELLOW);
    for (int i=0;i<8;i++) {
      float a = radians(i * 45.0 - 90.0);
      int x1 = cx + cos(a) * (iconSize/4 + 4);
      int y1 = cy + sin(a) * (iconSize/4 + 4);
      int x2 = cx + cos(a) * (iconSize/2);
      int y2 = cy + sin(a) * (iconSize/2);
      tft.drawLine(x1,y1,x2,y2, COL_YELLOW);
    }
  } else if (grp == 2 || grp == 3 || grp == 5) {
    // Rain: cloud + drops
    tft.fillCircle(cx - 6, cy - 4, iconSize/6, COL_LIGHTGR);
    tft.fillCircle(cx + 6, cy - 4, iconSize/6, COL_LIGHTGR);
    tft.fillRect(cx - 18, cy - 4, 36, iconSize/6, COL_LIGHTGR);
    // raindrops
    tft.drawLine(cx - 6, cy + 6, cx - 6, cy + 12, COL_CYAN);
    tft.drawLine(cx, cy + 6, cx, cy + 12, COL_CYAN);
    tft.drawLine(cx + 6, cy + 6, cx + 6, cy + 12, COL_CYAN);
  } else if (grp == 6) {
    // Snow: cloud + dots
    tft.fillCircle(cx - 6, cy - 4, iconSize/6, COL_LIGHTGR);
    tft.fillCircle(cx + 6, cy - 4, iconSize/6, COL_LIGHTGR);
    tft.fillRect(cx - 18, cy - 4, 36, iconSize/6, COL_LIGHTGR);
    tft.drawPixel(cx - 6, cy + 8, COL_WHITE);
    tft.drawPixel(cx, cy + 10, COL_WHITE);
    tft.drawPixel(cx + 6, cy + 8, COL_WHITE);
  } else {
    // Cloud default
    tft.fillCircle(cx - 6, cy - 4, iconSize/6, COL_LIGHTGR);
    tft.fillCircle(cx + 6, cy - 4, iconSize/6, COL_LIGHTGR);
    tft.fillRect(cx - 18, cy - 4, 36, iconSize/6, COL_LIGHTGR);
  }

  // Temperature text to the right of icon
  tft.setTextSize(2);
  tft.setTextColor(COL_YELLOW);
  char tbuf[12];
  if (isnan(curTemp)) {
    strcpy(tbuf, "--.-C");
  } else {
    snprintf(tbuf, sizeof(tbuf), "%.1fC", curTemp);
  }
  tft.setCursor(iconX + iconSize + 6, iconY + 6);
  tft.print(tbuf);

  // Weather description (short) below temp
  tft.setTextSize(1);
  tft.setTextColor(COL_CYAN);
  tft.setCursor(iconX + iconSize + 6, iconY + 22);
  if (curWeatherDesc.length() > 0) {
    // Shorten if too long
    String s = curWeatherDesc;
    if (s.length() > 14) s = s.substring(0, 14) + ".";
    tft.print(s);
  } else {
    tft.print("N/A");
  }
}

void drawDate() {
  // Build date string from timeClient epoch (timeClient already adjusted to JST)
  time_t epoch = timeClient.getEpochTime();
  struct tm *tmr = localtime(&epoch);
  char buf[32];
  // Example: "Fri, Nov 07 2025"
  strftime(buf, sizeof(buf), "%a, %b %d %Y", tmr);

  // Clear small area at bottom inside circle
  int16_t bx = CX - 80;
  int16_t by = CY + RADIUS - 42;
  tft.fillRect(bx, by, 160, 24, COL_BLACK);

  tft.setTextSize(1);
  tft.setTextColor(COL_WHITE);
  // center the date: approximate char width = 6 px at size 1
  int16_t textW = strlen(buf) * 6;
  int16_t tx = CX - textW / 2;
  tft.setCursor(tx, by + 4);
  tft.print(buf);
}

// -------------------- Weather fetch --------------------
void fetchWeather() {
  if (WiFi.status() != WL_CONNECTED) return;

  WiFiClient client;
  ESP8266HTTPClient http;
  String url = String("http://api.openweathermap.org/data/2.5/weather?q=") + CITY_NAME +
               "&units=metric&appid=" + OWM_API_KEY;

  Serial.print("Fetching weather: ");
  Serial.println(url);

  http.begin(client, url);
  int code = http.GET();
  if (code > 0 && code == HTTP_CODE_OK) {
    String payload = http.getString();
    Serial.println("Weather payload:");
    Serial.println(payload);

    // parse JSON
    const size_t CAP = 2048;
    DynamicJsonDocument doc(CAP);
    DeserializationError err = deserializeJson(doc, payload);
    if (!err) {
      if (doc.containsKey("main") && doc["main"].containsKey("temp")) {
        curTemp = doc["main"]["temp"].as<float>();
      }
      if (doc.containsKey("weather") && doc["weather"].size() > 0) {
        curWeatherId = doc["weather"][0]["id"].as<int>();
        if (doc["weather"][0].containsKey("description"))
          curWeatherDesc = String((const char*)doc["weather"][0]["description"]);
      }
      Serial.printf("Temp: %.1f  id=%d desc=%s\n", curTemp, curWeatherId, curWeatherDesc.c_str());
    } else {
      Serial.println("JSON parse error");
    }
  } else {
    Serial.printf("HTTP error: %d\n", code);
  }
  http.end();
}
