#ifndef __LedGroup__
#define __LedGroup__

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

    Adafruit_NeoPixel* strip;
    //Adafruit_NeoPixel* strip;

    void setNextColors();
    void setNextPixelColor(int i);

    int start;
    int length;

    int percentHidden();

    bool pixelHasTargetColor(byte);
    bool isPixelAnimated(byte);
    bool isHidden(byte);
    bool isAnimation(Animation);

    bool animationDone;
    
    Color getColor(byte);
    
    Color targetColors[10];
    byte pixelState[10];
    byte increment[10][3];
    int animatingPixel[10];
    int selectionLen;
    
    int hidePercent;
    byte counter;
};


#endif // __LedGroup__

