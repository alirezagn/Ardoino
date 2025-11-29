// Bodmer/TFT_eSPI  User_Setup.h  for ESP8266 + 240×240 ST7789
// Copy this file into  …/Arduino/libraries/TFT_eSPI/

#define USER_SETUP_INFO "ESP8266_ST7789_240x240"

// 1) DRIVER -------------------------------------------------
#define ST7789_DRIVER      // activate driver
#define ST7789_2_DRIVER    // 240×240 variant (bottom 80 rows)

// 2) SCREEN SIZE ------------------------------------------
#define TFT_WIDTH  240
#define TFT_HEIGHT 240

// 3) PINS ---------------------------------------------------
#define TFT_MISO -1        // not used
#define TFT_MOSI 13        // D7
#define TFT_SCLK 14        // D5
#define TFT_CS   15        // D8
#define TFT_DC    4        // D2
#define TFT_RST  16        // D0  (or -1 to tie to NodeMCU RST)
#define TFT_BL    5        // D1  (back-light)

// 4) OFFSETS FOR 240×240 PANEL -----------------------------
#define TFT_OFFSET_X 0
#define TFT_OFFSET_Y 80

// 5) COLOUR ORDER ------------------------------------------
#define TFT_RGB_ORDER TFT_BGR   // swap to RGB if colours inverted

// 6) SPI SPEED ---------------------------------------------
#define SPI_FREQUENCY  27000000   // 27 MHz is safe on breadboard
#define SPI_READ_FREQUENCY 20000000
#define SPI_TOUCH_FREQUENCY 2500000

// 7) COMMENT OUT EVERY OTHER SETUP FILE --------------------
//   (already done if you only left this file enabled in
//    User_Setup_Select.h)
