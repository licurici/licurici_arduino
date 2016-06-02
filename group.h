#ifndef __LedGroup__
#define __LedGroup__

//#include <Adafruit_WS2801.h>

#include <Adafruit_NeoPixel.h>

#include "color.h"

class LedGroup;

typedef void (* Animation) (LedGroup* group);
typedef void (* SelectionStrategy) (LedGroup* group);

class LedGroup {

  public:
    void setup(Adafruit_NeoPixel* strip, int start, int length);

    void animate();

    Animation animation;
    SelectionStrategy selection;
    int waitFrames;

    //Adafruit_WS2801* strip;
    
    Adafruit_NeoPixel* strip;
    int start;
    int length;

    int percentHidden();

    bool reachedTarget(byte);
    bool isSelected(byte);
    bool isHidden(byte);

    bool animationDone;
    
    Color getColor(byte);
    
    Color targetColors[10];
    byte state[10];
    byte increment[10][3];
    int selected[10];
    int selectionLen;
    
    int hidePercent;
    byte counter;
};


#endif // __LedGroup__
