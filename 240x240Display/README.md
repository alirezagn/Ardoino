# 240x240 ST7789 Display with ESP8266

This project provides a configuration and example sketch for using a 240x240 ST7789 TFT display with an ESP8266 microcontroller.

## Project Overview

This project contains:
*   A configuration file for a 240x240 TFT display using the ST7789 driver, intended for use with an ESP8266 microcontroller. The configuration file `tft_config/User_Setup.h` contains pin definitions and display settings.
*   The `display_hello.ino` sketch demonstrates how to use the display by showing "Hello" on the screen.

## Cabling

Here's the cabling list for your ST7789 display with an ESP8266, based on the `tft_config/User_Setup.h` configuration:

*   **BLK (Backlight):** ESP8266 GPIO5 (D1)
*   **DC (Data/Command):** ESP8266 GPIO4 (D2)
*   **RES (Reset):** ESP8266 GPIO16 (D0)
*   **SDA (MOSI - Data):** ESP8266 GPIO13 (D7) (Standard hardware SPI MOSI)
*   **SCK (Clock):** ESP8266 GPIO14 (D5) (Standard hardware SPI SCK)
*   **VCC:** ESP8266 3.3V
*   **GND:** ESP8266 GND

**Note on CS (Chip Select):** The `User_Setup.h` indicates that CS is not explicitly defined, allowing the library to use hardware SPI. This often means the library manages it internally or the display module has CS tied to ground. If your display module has a CS pin, it's typically connected to ESP8266 GPIO15 (D8) for hardware SPI, but the current configuration suggests it might not be necessary to define it.

## Building and Running

This project uses the [TFT_eSPI library](https://github.com/Bodmer/TFT_eSPI).

**To run the `display_hello.ino` sketch:**

1.  **Install the TFT_eSPI library:** If you haven't already, install the TFT_eSPI library in your Arduino IDE.
2.  **Configure `User_Setup.h`:**
    *   Copy the `tft_config/User_Setup.h` file into the TFT_eSPI library folder (e.g., `Arduino/libraries/TFT_eSPI/User_Setups/`).
    *   In the `User_Setup.h` file within the TFT_eSPI library, ensure that `User_Setup_Select.h` is configured to use your custom `User_Setup.h` (e.g., by uncommenting `#include <User_Setups/User_Setup.h>`).
3.  **Open and Upload:** Open `display_hello.ino` in the Arduino IDE, select your ESP8266 board, and upload the sketch.

## Development Conventions

The code follows standard Arduino/C++ conventions for sketches and header files.
