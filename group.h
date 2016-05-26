#ifndef __LedGroup__
#define __LedGroup__

#include <Adafruit_WS2801.h>
#include "color.h"

class LedGroup;

typedef void (* Animation) (LedGroup* group);
typedef void (* SelectionStrategy) (LedGroup* group);

class LedGroup {

  public:
    LedGroup(Adafruit_WS2801* strip, int start, int length);

    void animate();

    Animation animation;
    SelectionStrategy selection;
    int waitFrames;

    Adafruit_WS2801* strip;
    int start;
    int length;

    bool reachedTarget(byte);
    bool isSelected(byte);
    bool isHidden(byte);
    
    Color getColor(byte);
    
    Color targetColors[10];
    byte state[10];
    byte increment[10][3];
    int selected[10];
    int selectionLen;
    
    int hidePercent;

    private:
      byte counter;
};


#endif // __LedGroup__
