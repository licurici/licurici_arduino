#include "group.h"

void LedGroup::setup(Adafruit_NeoPixel* strip, int start, int length) {
  this->start = start;
  this->length = length;
  this->strip = strip;

  selectionLen = 0;
}

int LedGroup::percentHidden() {
  double countHidden = 0;
  int percentHidden = 0;

  for(int i=0; i<length; i++) {
    if(isHidden(i)) {
      countHidden++;
    }
  }

  return (countHidden / length) * 100;
}


void LedGroup::animate() {
  if(waitFrames > 0) {
    waitFrames--;
    return;
  }
  
  if(counter == 0 && selection != NULL) {
    selection(this);
    counter++;
  }

  if(animation != NULL) {
    animation(this);
  }
    
  for(int i=0; i<selectionLen; i++) {
    if(selected[i] > 4) {
      Color color = strip->getPixelColor(selected[i] + start);
      byte r = nextColor(Red(color), Red(targetColors[i]), increment[i][0]);
      byte g = nextColor(Green(color), Green(targetColors[i]), increment[i][0]);
      byte b = nextColor(Blue(color), Blue(targetColors[i]), increment[i][0]);
  
      strip->setPixelColor(selected[i] + start, createColor(r, g, b));
    }
  }
}

bool LedGroup::reachedTarget(byte index) {
  Color color = strip->getPixelColor(start + selected[index]);

  return color == targetColors[index];
}

bool LedGroup::isSelected(byte index) {

  for(int i=0; i<selectionLen; i++) {
    if(selected[i] == index) return true; 
  }
  
  return false; 
}

bool LedGroup::isHidden(byte index) {  
  return strip->getPixelColor(start + index) == 0;
}

Color LedGroup::getColor(byte index) {  
  return strip->getPixelColor(start + index);
}

bool LedGroup::isAnimation(Animation animation) {
  return animation == this->animation;
}


