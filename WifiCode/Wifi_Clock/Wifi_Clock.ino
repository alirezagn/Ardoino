//NOTE:  some reminants from the original clock program remain  
 
// An experiment using the ESP-8266 to lock onto time server and display it on an OLED

//  Using board from BangGood as ESP-12E module
// 6-28-2017 working

// TO GET OLED WORKING I NEEDED TO MAKE CHANGES SHOWN HERE
//  https://github.com/structure7/fridgeTemps
// OLED_RESET=0, etc and it works
// ON OLED/ESP12 pin D1 to SCL  and pin D2 to SDA


//************* Wol Clock by Jon Fuge ******************************
//  http://www.instructables.com/id/Wol-Clock-ESP8266-12E-60-LED-WS2812B-Analogue-Digi/?ALLSTEPS
//************* Declare included libraries ******************************
#include <NTPClient.h>
#include <Time.h>
#include <TimeLib.h>
#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define SSD1306_128_64
#define OLED_RESET 0
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2

//Set your timezone in hours difference rom GMT
const int hours_Offset_From_GMT = 8;

//Set your wifi details so the board can connect and get the time from the internet
const char *ssid      = "AliHomeLan25";    //  your network SSID (name)
const char *password  = "Ali1351Reza"; // your network password

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

//************* Setup function for Wol_Clock ******************************



void setup() {
  Serial.begin(9600);
  Serial.println("Test of clock!!!!!");
  Serial.println("Test of clock!!!!!");
  Serial.println("Test of clock!!!!!");

  WiFi.begin(ssid, password); // Try to connect to WiFi

  while ( WiFi.status() != WL_CONNECTED )
    delay ( 500 ); // keep waiging until we successfully connect to the WiFi

  SetClockFromNTP(); // get the time from the NTP server with timezone correction

  Wire.begin(4, 5);
  Serial.begin(9600);

  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x32)
  display.display();
  delay(2000);
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(12, 9);  // over (X) then down (Y)
  display.println("Syncing V2");
  display.display();
  delay(2000);
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

  display.clearDisplay();

  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(12, 9);  // over (X) then down (Y)
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
  delay(1000);
}
