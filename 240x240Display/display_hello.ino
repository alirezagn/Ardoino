#include <TFT_eSPI.h> // Hardware-specific library

TFT_eSPI tft = TFT_eSPI();       // Invoke custom library

void setup(void) {
  Serial.begin(115200);
  Serial.println("Initializing TFT...");

  tft.init();
  tft.setRotation(0); // Set the rotation of the display (0-3)

  tft.fillScreen(TFT_BLACK); // Fill the screen with black background

  tft.setTextFont(4); // Set font size (1 to 8, 1 is smallest)
  tft.setTextColor(TFT_WHITE, TFT_BLACK); // Set text color to white, background to black

  // Calculate position to center "Hello"
  String text = "Hello";
  int x = (tft.width() - tft.textWidth(text)) / 2;
  int y = (tft.height() - tft.fontHeight()) / 2;

  tft.setCursor(x, y); // Set cursor to calculated position
  tft.print(text); // Print "Hello"
  Serial.println("Hello displayed!");
}

void loop() {
  // Nothing to do in the loop for a static display
}
