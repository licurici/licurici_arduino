#include "group.h"

LedGroup::LedGroup(Adafruit_WS2801* strip, int start, int length) {
  this->start = start;
  this->length = length;
  this->strip = strip;
}


