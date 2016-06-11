#ifndef __GroupAnimation__
#define __GroupAnimation__

#include "group.h"

void setCurrentColor(Color color);
Color getCurrentColor();

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

char* stringAnimation(LedGroup* group);


#endif // __GroupAnimation__

