# Project Overview

This project is an Arduino-based clock for a round display (GC9A01A) powered by an ESP8266 microcontroller. It displays an analog clock face with the current time fetched from an NTP server and also shows current weather information from the OpenWeatherMap API.

The main source code is located in `round_display/round_display.ino`. An alternative or older version of the sketch is present in `sketch_nov9a/sketch_nov9a.ino`.

# Key Files

- `round_display/round_display.ino`: The main application sketch.
- `sketch_nov9a/sketch_nov9a.ino`: An alternative or older version of the application sketch.
- `wiring_guide.md`: Provides instructions for wiring the hardware components.
- `README.md`: Contains a general overview of the project, features, and setup instructions.

# Building and Running

## 1. Hardware Setup

- ESP8266 Development Board
- GC9A01A Round Display Module

Wire the components as described in `wiring_guide.md`.

## 2. Software Setup

- [Arduino IDE](https://www.arduino.cc/en/software)
- [ESP8266 Board Support for Arduino IDE](https://github.com/esp8266/Arduino)
- Required Libraries:
    - `Adafruit GFX`
    - `Adafruit GC9A01A`
    - `NTPClient`
    - `ArduinoJson`

## 3. Configuration

Before uploading the sketch, configure the following variables in `round_display/round_display.ino`:

```cpp
// ----------------- WIFI & NTP -----------------
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 9 * 3600, 60000); // JST (UTC+9)

// ----------------- WEATHER -----------------
const char* weatherUrl = "http://api.openweathermap.org/data/2.5/weather?q=YOUR_CITY,COUNTRY_CODE&appid=YOUR_OWM_API_KEY&units=metric";
```

**Note:** Replace `YOUR_WIFI_SSID`, `YOUR_WIFI_PASSWORD`, `YOUR_CITY`, `COUNTRY_CODE`, and `YOUR_OWM_API_KEY` with your actual credentials and location information.

## 4. Upload

- Open `round_display/round_display.ino` in the Arduino IDE.
- Select the correct board and port for your ESP8266.
- Upload the sketch.

# Development Conventions

The code is written in C++ for the Arduino platform. It uses several external libraries for display, networking, and data parsing. The code is structured with a `setup()` function for initialization and a `loop()` function for the main program logic. Helper functions are used to modularize tasks like drawing the clock face, fetching weather data, and drawing the clock hands.
