# ESP8266 Round Display Clock

This project is an Arduino-based clock for a round display (GC9A01A) powered by an ESP8266 microcontroller. It displays an analog clock face with the current time fetched from an NTP server and also shows current weather information from the OpenWeatherMap API.

## Features

*   Analog clock face with hour, minute, and second hands.
*   Weather information from OpenWeatherMap, including temperature and a description of the weather.
*   Partial redraw of the clock face to minimize flicker.
*   Automatic time synchronization with an NTP server.
*   Easy to configure Wi-Fi and OpenWeatherMap API credentials.

## Hardware

*   ESP8266 Development Board
*   GC9A01A Round Display Module

## Wiring

Please refer to the `wiring_guide.md` file for the wiring instructions.

## Software

*   [Arduino IDE](https://www.arduino.cc/en/software)
*   [ESP8266 Board Support for Arduino IDE](https://github.com/esp8266/Arduino)
*   Libraries:
    *   `Adafruit GFX`
    *   `Adafruit GC9A01A`
    *   `NTPClient`
    *   `ArduinoJson`

## Configuration

Before uploading the sketch to your ESP8266, you need to configure the following variables in the `round_display.ino` file:

*   `WIFI_SSID`: Your Wi-Fi network name.
*   `WIFI_PASS`: Your Wi-Fi password.
*   `OWM_API_KEY`: Your OpenWeatherMap API key.
*   `CITY_NAME`: The city for which you want to fetch weather data.
*   `GMT_OFFSET_SECONDS`: Your time zone offset in seconds.

## How it Works

The `round_display.ino` sketch is the heart of this project. It performs the following tasks:

1.  **Initialization:**
    *   Initializes the serial communication for debugging.
    *   Connects to the Wi-Fi network using the configured credentials.
    *   Initializes the GC9A01A display.
    *   Initializes the NTP client to fetch the current time.
    *   Fetches the initial weather data from the OpenWeatherMap API.
    *   Draws the static elements of the clock face, such as the rim and the hour ticks.

2.  **Main Loop:**
    *   Updates the time from the NTP client.
    *   Periodically fetches the weather data from the OpenWeatherMap API.
    *   Clears the inner area of the clock face.
    *   Draws the hour, minute, and second hands.
    *   Draws the date and the weather information.

## User Guide

1.  **Assembly:**
    *   Connect the ESP8266 and the GC9A01A display as described in the `wiring_guide.md` file.

2.  **Configuration:**
    *   Open the `round_display/round_display.ino` sketch in the Arduino IDE.
    *   Modify the following variables to match your settings:
        *   `WIFI_SSID`: Your Wi-Fi network name.
        *   `WIFI_PASS`: Your Wi-Fi password.
        *   `OWM_API_KEY`: Your OpenWeatherMap API key.
        *   `CITY_NAME`: The city for which you want to fetch weather data.
        *   `GMT_OFFSET_SECONDS`: Your time zone offset in seconds.

3.  **Upload:**
    *   Select the correct board and port for your ESP8266 in the Arduino IDE.
    *   Upload the sketch to your ESP8266.

4.  **Operation:**
    *   Once the sketch is uploaded, the ESP8266 will connect to the Wi-Fi network and display the current time and weather information on the round display.
    *   The time and weather information will be updated automatically.
