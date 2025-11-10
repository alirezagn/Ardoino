This document provides the wiring guide for the round display project.

**Components:**

*   ESP8266 Development Board
*   GC9A01A Round Display Module

**Wiring:**

Connect the components as follows:

| ESP8266 Pin | Wire Color | GC9A01A Pin |
| :---------- | :--------- | :---------- |
| D8          | <font color="blue">Blue</font>       | CS          |
| D1          | <font color="green">Green</font>     | DC          |
| D4          | <font color="yellow">Yellow</font>   | RST         |
| D5          | <font color="orange">Orange</font>   | SCLK        |
| D7          | <font color="purple">Purple</font>   | MOSI        |
| 3V3         | <font color="red">Red</font>         | VCC         |
| GND         | <font color="black">Black</font>     | GND         |

**Notes:**

*   The pin numbers on the ESP8266 board may vary. Please refer to the pinout diagram for your specific board.
*   The GC9A01A module may have additional pins, such as `BLK` (backlight) and `MISO`. The `BLK` pin can be connected to 3V3 to turn on the backlight. The `MISO` pin is not used in this project.
*   Ensure that the logic levels of the ESP8266 and the GC9A01A module are compatible. The GC9A01A module is typically 3.3V tolerant.
