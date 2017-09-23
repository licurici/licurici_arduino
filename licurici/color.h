#ifndef __color__
#define __color__

#include <Adafruit_NeoPixel.h>

typedef uint32_t Color;

// Create a 24 bit color value from R,G,B
Color createColor(byte r, byte g, byte b);

byte Blue(Color color);
byte Green(Color color);
byte Red(Color color);

byte nextColor(byte current, byte target, byte step);

#endif // __color__

