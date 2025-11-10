// User_Setup_GMT130.h

#define ST7789_DRIVER      // Use ST7789
#define TFT_WIDTH  240
#define TFT_HEIGHT 240

// --- ESP8266 Pin Definitions ---
// D2/GPIO4 for DC
#define TFT_DC    4      
// D0/GPIO16 for RES
#define TFT_RST   16     
// D1/GPIO5 for BLK
#define TFT_BL    5 
// The library will automatically use D5 (GPIO 14) for SCLK and D7 (GPIO 13) for MOSI.
// Since you used -1 for CS, we don't define it here, letting the library use hardware SPI.

// **CRITICAL OFFSET FIXES**
// These define the exact memory window needed for your 240x240 screen.
#define TFT_SDA_READ      // Some displays need this defined
#define TFT_INVERT_DISPLAY // Try inverting colors
#define TFT_RGB_ORDER TFT_BGR // Try BGR color order (often needed)
#define ST7789_2_AND_3 // Use this to tell the library it's a 240x240 on a larger chip

// Define the offset (required for 240x240 panels on 240x320 controllers)
#define TFT_SDA_READ
#define TFT_INVERT_DISPLAY
#define TFT_OFFSET_WIDTH 0 
#define TFT_OFFSET_HEIGHT 80 // Common offset for 240x240 displays