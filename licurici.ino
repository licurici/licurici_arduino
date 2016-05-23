#include <Adafruit_WS2801.h>
#include "group.h"

const uint8_t dataPin  = 2;    // Yellow wire on Adafruit Pixels
const uint8_t clockPin = 3;    // Green wire on Adafruit Pixels
const int ledCount = 25;

uint32_t desiredRed;
uint32_t desiredGreen;
uint32_t desiredBlue;

Adafruit_WS2801 strip = Adafruit_WS2801(ledCount, dataPin, clockPin);
LedGroup group = LedGroup(&strip, 0, 25);

void setup() {
  Serial.begin(9600);

  strip.begin();

  // Update LED contents, to start they are all 'off'
  strip.show();

  desiredRed = 255;
  desiredGreen = 0;
  desiredBlue = 255;
}

void loop() {
  showFireflies();
}

int* selectRandomPixels() {

  int* pixels = (int*) malloc(sizeof(int) * 5);

  for(int i = 0; i<5; i++) {
    pixels[i] = random(0, ledCount);
  }

  return pixels;  
}

void showFireflies() {

    int* pixels = selectRandomPixels();

    for(int i=0; i<50; i++) {
      for(int j=0; j<5; j++) {
        uint32_t currentColor = strip.getPixelColor(pixels[j]);
  
        byte r = Red(currentColor);
        byte g = Green(currentColor);
        byte b = Blue(currentColor);
 
        if(r < desiredRed) r++;
        if(g < desiredGreen) g++;
        if(b < desiredBlue) b++;
  
        Serial.print("=>");
        Serial.println(r);
  
        strip.setPixelColor(pixels[j], Color(r, g, b));
      }
      
      delay(20);
      strip.show();
    }

    free(pixels);
}

// Create a 24 bit color value from R,G,B
uint32_t Color(byte r, byte g, byte b)
{
  uint32_t c;
  c = r;
  c <<= 8;
  c |= g;
  c <<= 8;
  c |= b;
  return c;
}

// Create a 24 bit color value from R,G,B
byte Blue(uint32_t color)
{
  return (byte) color;
}

byte Green(uint32_t color)
{
  return (byte) (color >> 8);
}

byte Red(uint32_t color)
{
  return (byte) (color >> 16);
}

