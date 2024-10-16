
// WiFi Include librarie
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         // https://github.com/tzapu/WiFiManager
#include <WiFiUdp.h>

// OLED Include Libraries
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_NeoPixel.h>

// time libraries
#include <NTPClient.h>
#include <Time.h>
#include <TimeLib.h>



// set OLED width and height
#define SSD1306_128_64
#define OLED_RESET 0
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels


// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

//
//#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2

// 
//Set your timezone in hours difference rom GMT
const int hours_Offset_From_GMT = 8;
byte SetClock;

// By default 'time.nist.gov' is used.
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

// Which pin on the ESP8266 is connected to the NeoPixels?
#define PIN            14 // This is the D5 pin

//************* Declare user functions ******************************
void Draw_Clock(time_t t, byte Phase);
int ClockCorrect(int Pixel);
void SetBrightness(time_t t);
void SetClockFromNTP ();
bool IsDst();

//************* Declare NeoPixel ******************************
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(60, PIN, NEO_GRB + NEO_KHZ800);


void setup() {
  Serial.begin(9600);
  
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(2000);
  
  // WiFiManager
  // Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
  
  // Uncomment and run it once, if you want to erase all the stored information
  //wifiManager.resetSettings();
  
  // set custom ip for portal
  wifiManager.setAPStaticIPConfig(IPAddress(192,168,254,1), IPAddress(192,168,254,1), IPAddress(255,255,255,0));
  // fetches ssid and pass from eeprom and tries to connect
  // if it does not connect it starts an access point with the specified name
  // here  "WiFiClock"
  // and goes into a blocking loop awaiting configuration
  
  wifiManager.autoConnect("WiFiClock");
  // or use this for auto generated name ESP + ChipID
  //wifiManager.autoConnect();
  


  // Display AP information to configure WiFi SSD / Password
  display.clearDisplay();
  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.setCursor(0, 0);     // Start at top-left corner
  display.println("WiFi Not Setup...");
  display.println("Connect WiFiClock to setup");
  display.display();
  delay(2000); // Pause for 2 seconds
  
  // Display WiFi information 
  // Clear the buffer
  display.clearDisplay();
  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.setCursor(0, 0);     // Start at top-left corner
  display.println("Connected...");
  display.display();
  delay(2000);
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("SSID:");
  display.println(wifiManager.getWiFiSSID());
  display.println(WiFi.localIP());
  display.display();
  delay(2000);
  
  // if you get here you have connected to the WiFi
  Serial.println("Connected.");

  //*****************************
  // start display clock 
  //*****************************
  SetClockFromNTP(); // get the time from the NTP server with timezone correction

  Wire.begin(4, 5);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(5, 9);  // over (X) then down (Y)
  display.println("WiFi Clock V1");
  display.display();
  delay(2000);

  server.begin();
}

void SetClockFromNTP ()
{
  timeClient.update(); // get the time from the NTP server
  setTime(timeClient.getEpochTime()); // Set the system yime from the clock
  if (IsDst())
    adjustTime((hours_Offset_From_GMT + 1) * 3600); // offset the system time with the user defined timezone (3600 seconds in an hour)
  else
    adjustTime(hours_Offset_From_GMT * 3600); // offset the system time with the user defined timezone (3600 seconds in an hour)
}

bool IsDst()
{
  if (month() < 3 || month() > 10)  return false;
  if (month() > 3 && month() < 10)  return true;
  int previousSunday = day() - weekday();
  if (month() == 3) return previousSunday >= 24;
  if (month() == 10) return previousSunday < 24;

  return false; // this line never gonna happend
}

//************* Main program loop for Wol_Clock ******************************
void loop() {
  // put your main code here, to run repeatedly:

  time_t t = now(); // Get the current time
  
  SetBrightness(t);
  if (minute(t) == 0) // at the start of each hour, update the time from the time server
    if (SetClock == 1)
    {
      SetClockFromNTP(); // get the time from the NTP server with timezone correction
      SetClock = 0;
    }
    else
    {
      delay(200); // Just wait for 0.1 seconds
      SetClock = 1;
    }
}


//************* Function to set the clock brightness ******************************
void SetBrightness(time_t t)
{
  int NowHour = hour(t);
  int NowMinute = minute(t);
  int NowSecond = second(t);
  Serial.print("Hour = ");
  Serial.print(NowHour);
  Serial.print(" Minute= "); Serial.print(NowMinute);
  Serial.print(" Second= "); Serial.println(NowSecond);
  Serial.print(day()); Serial.print("/"); Serial.print(month()); Serial.print("/"); Serial.println(year());

  display.clearDisplay();

  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(15, 2);  // over (X) then down (Y)
  display.print(NowHour);
  display.print(":");
  if (NowMinute <= 9) {
    display.print("0");  // pad #s < 10 with leading zero
  }
  display.print(NowMinute);
  display.print(":");
  if (NowSecond <= 9) {  // pad #s < 10 with leading zero
    display.print("0");
  }
  display.print(NowSecond);
  display.display();

  // Display date
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(15, 20);  // over (X) then down (Y)
  display.print(year());
  display.print("/");
  display.print(month());
  display.print("/");
  display.print(day());

  display.display();
  delay(1000);
}
