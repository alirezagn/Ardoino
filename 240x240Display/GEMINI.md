# Project Overview

This project contains a configuration file for a 240x240 TFT display using the ST7789 driver, intended for use with an ESP8266 microcontroller. The configuration file `tft_config/User_Setup.h` contains pin definitions and display settings.

The `display_hello.ino` sketch demonstrates how to use the display by showing "Hello" on the screen.

# Building and Running

This project uses the [TFT_eSPI library](https://github.com/Bodmer/TFT_eSPI).

**To run the `display_hello.ino` sketch:**

1.  **Install the TFT_eSPI library:** If you haven't already, install the TFT_eSPI library in your Arduino IDE.
2.  **Configure `User_Setup.h`:**
    *   Copy the `tft_config/User_Setup.h` file into the TFT_eSPI library folder (e.g., `Arduino/libraries/TFT_eSPI/User_Setups/`).
    *   In the `User_Setup.h` file within the TFT_eSPI library, ensure that `User_Setup_Select.h` is configured to use your custom `User_Setup.h` (e.g., by uncommenting `#include <User_Setups/User_Setup.h>`).
3.  **Open and Upload:** Open `display_hello.ino` in the Arduino IDE, select your ESP8266 board, and upload the sketch.

# Development Conventions

The code follows standard Arduino/C++ conventions for sketches and header files.
