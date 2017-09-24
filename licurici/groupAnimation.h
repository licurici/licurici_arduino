#ifndef __GroupAnimation__
#define __GroupAnimation__

#include "group.h"

void setFlickerIntensity(byte intensity);

void setCurrentColor(Color color);
Color getCurrentColor();
void randomColor();

void hilightFlicker(LedGroup* group);
void flicker(LedGroup* group);
void flickerStrategy(LedGroup* group);

void road(LedGroup* group);
void roadStrategy(LedGroup* group);

void hide(LedGroup* group);
void hideStrategy(LedGroup* group);

void happy(LedGroup* group);
void happyStrategy(LedGroup* group);

void show(LedGroup* group);
void showStrategy(LedGroup* group);

const char* stringAnimation(LedGroup* group);


#endif // __GroupAnimation__

