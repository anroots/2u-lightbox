
// 2u lightbox firmware
// https://github.com/anroots/2u-lightbox
// 2022-04-10; ando@sqroot.eu; MIT license

// See doc https://github.com/FastLED/FastLED/wiki/Pixel-reference
#include <FastLED.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

// LED setup
#define NUM_LEDS_PER_STRIP 24
#define LED_PIN1 4        // D2
#define LED_PIN2 5        // D1
// 750mA - Power limit in mA (voltage is set in setup() to 5v)
// Check your power supply. If powering from regular USB port, keep this well below 500mA.
#define LED_PWR_LIMIT 450

// WiFi setup - ESP will connect to it and ask for DHCP IP
const char *ssid = "REPLACEME";
const char *password = "REPLACEME";

ESP8266WebServer server(80);

int defaultBrightness = 150;
int brightness = defaultBrightness; // Initial brightness, 0-255
int color = 150;       // Initial color (from the color map below), 0-256 

// This is an array of leds.  One item for each led in your strip.
CRGB leds[NUM_LEDS_PER_STRIP];

// https://github.com/FastLED/FastLED/wiki/Pixel-reference
CRGBPalette16 currentPalette = CRGBPalette16(
    CRGB::White,        // 0-16
    CRGB::Amethyst,     // 17-32
    CRGB::Brown,        // 33-48
    CRGB::Coral,        // 49-64
    CRGB::Green,        // 65-80
    CRGB::DarkOrange,   // 81-96
    CRGB::Gold,         // 97-112
    CRGB::DarkSeaGreen, // 113-128
    CRGB::HotPink,      // 129-144
    CRGB::DodgerBlue,   // 145-160
    CRGB::LightGreen,   // 161-176
    CRGB::Lime,         // 177-192
    CRGB::OldLace,      // 193-208
    CRGB::Silver,       // 209-224
    CRGB::DarkRed,      // 225-240
    CRGB::Black         // 241-256
);

void setup()
{
   // sanity check delay - allows reprogramming if accidently blowing power w/leds
   delay(2000);

   // Our setup has two separate LED strips
   FastLED.addLeds<WS2812B, LED_PIN1, GRB>(leds, NUM_LEDS_PER_STRIP);
   FastLED.addLeds<WS2812B, LED_PIN2, GRB>(leds, NUM_LEDS_PER_STRIP);

   FastLED.setMaxPowerInVoltsAndMilliamps(5, LED_PWR_LIMIT);
   FastLED.setBrightness(brightness);
   FastLED.clear();

   WiFi.hostname("2u-lightbox");
   WiFi.begin(ssid, password);

   // Wait for WiFi connection, give loading indicator via LEDs
   int whiteLed = 0;
   while (WiFi.status() != WL_CONNECTED)
   {
      leds[whiteLed] = CRGB::Black;

      whiteLed += 1;
      if (whiteLed >= NUM_LEDS_PER_STRIP)
      {
         whiteLed = 0;
      }
      leds[whiteLed] = CRGB::White;
      FastLED.show();

      delay(500);
   }

   // Setup HTTP API handlers
   server.on("/", handleRoot);
   server.on("/brightness", handleBrightnessSet);
   server.on("/color", handleColorSet);
   server.on("/state", handleStateSet);
   server.onNotFound(handleNotFound);
   server.begin();

   turnOn();
   refresh();
}

void handleNotFound()
{
   String message = "No cookies here\n\n";
   server.send(404, "text/plain", message);
}

void handleRoot()
{
   String message = "{\"help\":\"2u lightbox API\", \"brightness\":";
   message += brightness;
   message += ", \"color\":";
   message += color;
   message += "}";

   server.send(200, "application/json", message);
}

void handleBrightnessSet()
{
   if (server.method() != HTTP_POST)
   {
      server.send(405, "text/plain", "Method Not Allowed");
      return;
   }

   String brightnessInput = server.arg("brightness");
   int newBrightness = brightnessInput.toInt();

   if (newBrightness > 255)
   {
      server.send(400, "text/plain", "Invalid input");
      return;
   }
   brightness = newBrightness;
   refresh();

   server.send(200, "text/plain", "Brightness set " + brightnessInput);
}


void handleStateSet()
{
   if (server.method() != HTTP_POST)
   {
      server.send(405, "text/plain", "Method Not Allowed");
      return;
   }

   String newState = server.arg("state");

   if (newState == "off") {
      brightness = 0;
   } else {
      brightness = defaultBrightness;      
   }
   
   refresh();

   server.send(200, "text/plain", "ok");
}

void handleColorSet()
{
   if (server.method() != HTTP_POST)
   {
      server.send(405, "text/plain", "Method Not Allowed");
      return;
   }
   String colorInput = server.arg("color");
   int newColor = colorInput.toInt();

   if (newColor > 255)
   {
      server.send(400, "text/plain", "Invalid input");
      return;
   }
   color = newColor;
   refresh();
   server.send(200, "text/plain", "Color set " + colorInput);
}

void refresh()
{

   FastLED.setBrightness(brightness);

   if (brightness == 0)
   {
      FastLED.clear();
      FastLED.show();
      return;
   }
   colorize(color);
}

void turnOn()
{

   FastLED.clear();
   int current = 0;
   while (true)
   {

      current += 1;
      if (current >= NUM_LEDS_PER_STRIP)
      {
         break;
      }
      leds[current] = CRGB::AliceBlue;
      FastLED.show();

      delay(80);
   }

   while (true)
   {

      current += 3;
      if (current >= 252)
      {
         break;
      }
      colorize(current);
   }
}

void knightRider()
{

   int current = 0;
   bool direction = true;

   while (true)
   {

      if (direction)
      {
         current += 1;
      }
      else
      {
         current -= 1;
      }
      if (current >= NUM_LEDS_PER_STRIP || current == 0)
      {
         direction = !direction;
      }
      leds[current] = CRGB::FireBrick;
      FastLED.show();
      delay(100);
      leds[current] = color;
      FastLED.show();
      delay(100);
      server.handleClient();
   }
}

void colorize(int colorIndex)
{
   int current = 0;
   while (true)
   {

      if (current >= NUM_LEDS_PER_STRIP)
      {
         break;
      }
      leds[current] = ColorFromPalette(currentPalette, colorIndex, brightness, LINEARBLEND);
      FastLED.show();

      current += 1;
      delay(10);
   }
}


void loop()
{

   server.handleClient();
   refresh();
   //knightRider();

   delay(1000);
}
