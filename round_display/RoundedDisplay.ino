ication #include <Adafruit_GC9A01A.h>

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Adafruit_GFX.h>

#include <SPI.h>


// ====== Wi-Fi Config ======
const char* ssid = "AlirezaHome_24";
const char* password = "Ali1351Reza";

// ====== OpenWeatherMap Config ======
const String city = "Tokyo";
const String apiKey = "364a1c2935392f2fd5fe1b2a91845fab"; // Replace with your OpenWeatherMap API key
const String apiURL = "http://api.openweathermap.org/data/2.5/weather?q=" + city + "&appid=" + apiKey + "&units=metric";

// ====== Display Pins (adjust if needed) ======
#define TFT_CS   D8
#define TFT_DC   D3
#define TFT_RST  D4
#define TFT_SCLK D5
#define TFT_MOSI D7

Adafruit_GC9A01A tft(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);
WiFiClient client;

// ====== Function to Fetch Weather ======
void updateWeather() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected!");
    return;
  }

  HTTPClient http;
  http.begin(client, apiURL); // ✅ fixed deprecated begin()

  int httpCode = http.GET();
  if (httpCode > 0) {
    String payload = http.getString();
    Serial.println(payload);
    tft.fillScreen(GC9A01A_BLACK);q
    tft.setCursor(10, 40);
    tft.setTextSize(2);
    tft.setTextColor(GC9A01A_WHITE);
    tft.println("Weather in " + city);
    tft.setTextSize(1);
    tft.println(payload.substring(0, 200)); // print partial JSON
  } else {
    Serial.println("Error on HTTP request");
    tft.setCursor(10, 40);
    tft.println("Error fetching data");
  }

  http.end();
}

// ====== Setup ======
void setup() {
  Serial.begin(115200);
  tft.begin();
  tft.setRotation(0);
  tft.fillScreen(GC9A01A_BLACK);
  tft.setTextColor(GC9A01A_WHITE);
  tft.setTextSize(2);
  tft.setCursor(20, 100);
  tft.println("Connecting WiFi...");

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  tft.fillScreen(GC9A01A_BLACK);
  tft.setCursor(40, 100);
  tft.println("WiFi Connected!");
  delay(1000);
}

// ====== Loop ======
void loop() {
  updateWeather();
  delay(60000); // update every 60 seconds
}
