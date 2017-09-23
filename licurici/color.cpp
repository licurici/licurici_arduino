#include "color.h"

// Create a 24 bit color value from R,G,B
Color createColor(byte r, byte g, byte b)
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

byte nextColor(byte current, byte target, byte step) {
  if(current < target && current + step > current) {
    return max(min(current + step, target), 0);
  }

  if(current > target && current - step < current) {
    return min(max(current - step, target), 250);
  }
  
  return current;
}


