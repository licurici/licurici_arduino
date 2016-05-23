#ifndef __LedGroup__
#define __LedGroup__

#include <Adafruit_WS2801.h>

class LedGroup {

  public:
    LedGroup(Adafruit_WS2801* strip, int start, int length);


  private:
    Adafruit_WS2801* strip;
    int start;
    int length;
};

#endif // __LedGroup__
