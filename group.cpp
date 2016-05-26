#include "group.h"



LedGroup::LedGroup(Adafruit_WS2801* strip, int start, int length) {
  this->start = start;
  this->length = length;
  this->strip = strip;

  selectionLen = 0;
}


void LedGroup::animate() {
  if(waitFrames > 0) {
    waitFrames--;
    return;
  }
  
  if(counter == 0) {
    selection(this);
    counter++;
  }

  animation(this);
    
  for(int i=0; i<selectionLen; i++) {
    Color color = strip->getPixelColor(selected[i]);
    byte r = nextColor(Red(color), Red(targetColors[i]), increment[i][0]);
    byte g = nextColor(Green(color), Green(targetColors[i]), increment[i][0]);
    byte b = nextColor(Blue(color), Blue(targetColors[i]), increment[i][0]);

    strip->setPixelColor(selected[i] + start, createColor(r, g, b));
  }
}

bool LedGroup::reachedTarget(byte index) {
  Color color = strip->getPixelColor(selected[index]);

  return color == targetColors[index];
}

bool LedGroup::isSelected(byte index) {

  for(int i=0; i<selectionLen; i++) {
    if(selected[i] == index) return true; 
  }
  
  return false; 
}
