/* ESP8266 STOCK MONITOR (YAHOO ONLY) + WEB CONFIG + IP BUTTON
   Hardware:
   - Display: ST7789 240x240
   - Button: Pin D2 to GND
*/

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include <LittleFS.h> 
#include <Fonts/FreeSansBold9pt7b.h> 
#include <Fonts/FreeSansBold12pt7b.h> // Added larger font for IP screen

// --- PINS ---
#define TFT_CS   D8
#define TFT_DC   D1
#define TFT_RST  D4
#define TFT_BL   D3
#define BTN_PIN  D2  // <-- NEW: Button Pin

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
ESP8266WebServer server(80);

// --- WIFI SETTINGS ---
const char* ssid     = "AlirezaHome_24";
const char* password = "Ali1351Reza";

// --- DATA STRUCTURE ---
struct Asset {
  String name;      // Display Name
  String ticker;    // Yahoo Ticker (e.g. "1357.T")
  
  // Live Data
  float price = 0;
  float rsi = 50;
  bool buy = false;
  bool sell = false;
} assets[4]; 

unsigned long lastUpdate = 0;
unsigned long lastBlink = 0;
bool blinkState = false;

// --- WEB PAGE HTML ---
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>Stock Config</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body { background-color: #121212; color: #e0e0e0; font-family: sans-serif; padding: 20px; }
    h2 { text-align: center; color: #00ff88; border-bottom: 2px solid #333; padding-bottom: 10px; }
    .card { background: #1e1e1e; border-radius: 8px; padding: 15px; margin-bottom: 15px; border: 1px solid #333; }
    .card h3 { margin-top: 0; color: #ffcc00; }
    label { display: block; margin-top: 10px; font-size: 0.9em; color: #aaa; }
    input { width: 100%; padding: 8px; margin-top: 5px; background: #2c2c2c; border: 1px solid #444; color: #fff; border-radius: 4px; box-sizing: border-box;}
    button { width: 100%; padding: 12px; background: #00ff88; color: #000; font-weight: bold; border: none; border-radius: 6px; cursor: pointer; margin-top: 10px; }
    .grid { display: grid; grid-template-columns: 1fr 1fr; gap: 10px; }
  </style>
</head><body>
  <h2>Stock Configuration</h2>
  <form action="/save" method="POST">
    <div class="grid">
      <div class="card"><h3>Top Left</h3>
        <label>Display Name</label><input type="text" name="n0" id="n0" maxlength="6">
        <label>Yahoo Ticker</label><input type="text" name="t0" id="t0">
      </div>
      <div class="card"><h3>Top Right</h3>
        <label>Display Name</label><input type="text" name="n1" id="n1" maxlength="6">
        <label>Yahoo Ticker</label><input type="text" name="t1" id="t1">
      </div>
      <div class="card"><h3>Bottom Left</h3>
        <label>Display Name</label><input type="text" name="n2" id="n2" maxlength="6">
        <label>Yahoo Ticker</label><input type="text" name="t2" id="t2">
      </div>
      <div class="card"><h3>Bottom Right</h3>
        <label>Display Name</label><input type="text" name="n3" id="n3" maxlength="6">
        <label>Yahoo Ticker</label><input type="text" name="t3" id="t3">
      </div>
    </div>
    <button type="submit">SAVE & RESTART</button>
  </form>
  <script>
    fetch('/config.json').then(r => r.json()).then(data => {
      for(let i=0; i<4; i++) {
        document.getElementById('n'+i).value = data[i].name;
        document.getElementById('t'+i).value = data[i].ticker;
      }
    });
  </script>
</body></html>
)rawliteral";

void loadConfig();
void saveConfig();
void fetchAll();
void drawAll();
void drawStatusDot(uint16_t color);
void showIPScreen(); // New function

void setup() {
  Serial.begin(115200);
  
  // 1. Init Hardware
  pinMode(TFT_BL, OUTPUT); digitalWrite(TFT_BL, HIGH);
  pinMode(BTN_PIN, INPUT_PULLUP); // Button setup
  
  // 2. Init FS
  if (!LittleFS.begin()) {
    LittleFS.format();
    LittleFS.begin();
  }

  // 3. Init Display
  tft.init(240, 240); tft.setRotation(0);
  tft.fillScreen(ST77XX_BLACK);
  tft.setFont(&FreeSansBold9pt7b);
  
  // 4. Connect WiFi
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(40, 100); tft.print("Connecting WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);

  // 5. Load Config
  loadConfig();

  // 6. Web Server Setup
  server.on("/", HTTP_GET, []() { server.send(200, "text/html", index_html); });
  server.on("/config.json", HTTP_GET, []() {
    File file = LittleFS.open("/config.json", "r");
    if (file) { server.streamFile(file, "application/json"); file.close(); } 
    else server.send(404, "text/plain", "Not found");
  });
  server.on("/save", HTTP_POST, []() {
    for (int i = 0; i < 4; i++) {
      assets[i].name = server.arg("n" + String(i));
      assets[i].ticker = server.arg("t" + String(i));
    }
    saveConfig();
    server.send(200, "text/html", "Saved! Restarting...");
    delay(500); ESP.restart();
  });
  server.begin();
  
  // Initial Fetch
  showIPScreen(); // Show IP once at startup
  delay(2000);
  
  drawStatusDot(ST77XX_RED); 
  fetchAll();
  drawAll();
}

void loop() {
  server.handleClient();
  
  // --- BUTTON LOGIC ---
  // If button is pressed (LOW)
  if (digitalRead(BTN_PIN) == LOW) {
    showIPScreen();
    // Wait here while user holds the button
    while(digitalRead(BTN_PIN) == LOW) {
       server.handleClient(); // Keep web server alive even while holding button
       delay(10);
    }
    // When released, redraw the stocks
    drawAll();
  }
  // --------------------

  unsigned long currentMillis = millis();

  if (currentMillis - lastUpdate > 60000) {
    drawStatusDot(ST77XX_MAGENTA);
    fetchAll();
    drawAll();
    lastUpdate = currentMillis;
  }

  if (currentMillis - lastBlink > 1000) {
    blinkState = !blinkState;
    drawStatusDot(blinkState ? 0x07E0 : 0x0000);
    lastBlink = currentMillis;
  }
  
  delay(50);
}

// --- NEW FUNCTION: SHOW IP ---
void showIPScreen() {
  tft.fillScreen(ST77XX_BLUE); // Blue background to stand out
  tft.setTextColor(ST77XX_WHITE);
  
  tft.setFont(&FreeSansBold12pt7b); // Larger font
  tft.setCursor(30, 100);
  tft.print("Config IP:");
  
  tft.setFont(&FreeSansBold9pt7b);
  tft.setCursor(45, 140);
  tft.print(WiFi.localIP());
}

// --- CONFIG ---
void loadConfig() {
  if (LittleFS.exists("/config.json")) {
    File file = LittleFS.open("/config.json", "r");
    StaticJsonDocument<1024> doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();
    if (!error) {
      for (int i = 0; i < 4; i++) {
        assets[i].name = doc[i]["name"].as<String>();
        assets[i].ticker = doc[i]["ticker"].as<String>();
      }
      return;
    }
  }
  // Defaults (All Stocks now)
  assets[0] = {"NVDA", "NVDA"};
  assets[1] = {"TSLA", "TSLA"};
  assets[2] = {"1357", "1357.T"};
  assets[3] = {"1570", "1570.T"};
  saveConfig();
}

void saveConfig() {
  StaticJsonDocument<1024> doc;
  for (int i = 0; i < 4; i++) {
    doc[i]["name"] = assets[i].name;
    doc[i]["ticker"] = assets[i].ticker;
  }
  File file = LittleFS.open("/config.json", "w");
  serializeJson(doc, file);
  file.close();
}

// --- VISUALS ---
void drawStatusDot(uint16_t color) {
  tft.fillCircle(120, 120, 4, color);
}

void drawYenSymbol(int x, int y, uint16_t color) {
  int w = 8; int h = 10;
  tft.drawLine(x, y, x + w/2, y + h/2, color);
  tft.drawLine(x + w, y, x + w/2, y + h/2, color);
  tft.drawLine(x + w/2, y + h/2, x + w/2, y + h, color);
  tft.drawFastHLine(x, y + h/2 - 2, w, color);
  tft.drawFastHLine(x, y + h/2 + 1, w, color);
}

void drawBlock(int x, int y, Asset &asset) {
  tft.setTextColor(ST77XX_YELLOW); 
  tft.setCursor(x + 10, y + 25); 
  tft.print(asset.name);

  drawYenSymbol(x + 10, y + 48, ST77XX_GREEN); 
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(x + 25, y + 60);
  if (asset.price > 10000) tft.printf("%.0f", asset.price);
  else tft.printf("%.1f", asset.price);

  tft.setCursor(x + 10, y + 95);
  if (asset.buy) {
    tft.setTextColor(ST77XX_GREEN); tft.print("BUY ");
    tft.setTextColor(ST77XX_WHITE); tft.print(String((int)asset.rsi));
  } else if (asset.sell) {
    tft.setTextColor(ST77XX_RED); tft.print("SELL ");
    tft.setTextColor(ST77XX_WHITE); tft.print(String((int)asset.rsi));
  } else {
    tft.setTextColor(ST77XX_CYAN); tft.print("RSI ");
    tft.setTextColor(ST77XX_WHITE); tft.print(String((int)asset.rsi));
  }
}

void drawAll() {
  tft.fillScreen(ST77XX_BLACK);
  tft.setFont(&FreeSansBold9pt7b);
  tft.drawFastVLine(120, 0, 240, 0x4A49); 
  tft.drawFastHLine(0, 120, 240, 0x4A49); 
  drawBlock(0, 0, assets[0]);
  drawBlock(120, 0, assets[1]);
  drawBlock(0, 120, assets[2]);
  drawBlock(120, 120, assets[3]);
  drawStatusDot(ST77XX_GREEN);
}

// --- DATA FETCHING (YAHOO ONLY) ---
void fetchAll() {
  WiFiClient client;
  
  for (int i = 0; i < 4; i++) {
    HTTPClient h;
    String url = "http://query1.finance.yahoo.com/v8/finance/chart/" + assets[i].ticker + "?interval=5m&range=1d";
    h.begin(client, url);
    h.setUserAgent("Mozilla/5.0");

    int code = h.GET();
    if (code == HTTP_CODE_OK) {
      StaticJsonDocument<200> filter;
      filter["chart"]["result"][0]["indicators"]["quote"][0]["close"] = true;
      filter["chart"]["result"][0]["meta"]["regularMarketPrice"] = true;

      DynamicJsonDocument doc(8192); // Buffer
      DeserializationError error = deserializeJson(doc, h.getStream(), DeserializationOption::Filter(filter));

      if (!error) {
        float gain = 0, loss = 0;
        int count = 0;
        float lastPrice = 0;

        JsonArray closes = doc["chart"]["result"][0]["indicators"]["quote"][0]["close"];
        int len = closes.size();
        if (len > 15) {
          for (int j = len - 15; j < len; j++) {
            float curr = closes[j].as<float>();
            float prev = closes[j-1].as<float>();
            if (curr == 0 || prev == 0) continue; 
            float diff = curr - prev;
            if (diff > 0) gain += diff; else loss -= diff;
            lastPrice = curr;
          }
          count = 14;
        }

        if (count > 0 && lastPrice > 0) {
          float rs = (loss == 0) ? 0 : (gain / loss);
          assets[i].rsi = (loss == 0) ? 100 : 100 - (100 / (1 + rs));
          assets[i].price = lastPrice;
          assets[i].buy = assets[i].rsi < 30;
          assets[i].sell = assets[i].rsi > 70;
        }
      }
    }
    h.end();
  }
}