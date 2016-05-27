#include <Adafruit_WS2801.h>
#include "group.h"
#include "groupAnimation.h"

const uint8_t dataPin  = 2;    // Yellow wire on Adafruit Pixels
const uint8_t clockPin = 3;    // Green wire on Adafruit Pixels
const int ledCount = 25;

Adafruit_WS2801 strip = Adafruit_WS2801(ledCount, dataPin, clockPin);

#define TOTAL_GROUPS 2

LedGroup groups[TOTAL_GROUPS];
 
enum SerialAction {
  showAction,
  flickerAction,
  roadAction,
  hideAction,
  unknownAction
};

void setup() {  
  delay(2000);

  Serial.begin(9600);

  strip.begin();

  // Update LED contents, to start they are all 'off'
  strip.show();
  
  Serial.println("Setup strips"); 
  groups[0].setup(&strip, 0, 10);
  //groups[0].animation = &flicker;
  //groups[0].selection = &flickerStrategy;

  groups[1].setup(&strip, 10, 15);
  groups[1].animation = &flicker;
  groups[1].selection = &flickerStrategy;

  
  Serial.println("Start loop"); 
}

void loop() {
  for(int i=0; i<TOTAL_GROUPS; i++)
    groups[i].animate();
  
  strip.show();

  delay(20);
  
  SerialAction action = unknownAction;

  if(Serial.available() > 0) {
    Serial.println("\n\nparse command");  
    action = intToAction(Serial.parseInt());

    if(action == unknownAction) {
      Serial.println("Unknown command");  
    }
  }

  if(action != unknownAction) {
    performAction(action);
    action = unknownAction;
  }
}

void performAction(SerialAction action) {
    int nr;
    Serial.setTimeout(5000);
    
    switch(action) {
      case showAction:
        Serial.println("Select show group");  
        nr = Serial.parseInt();

        groups[nr].animation = &show;
        groups[nr].selection = &showStrategy;
        groups[nr].counter = 0;
        groups[nr].waitFrames = 0;
        
        break;
        
      case flickerAction:
        Serial.println("Select flicker group");  
        nr = Serial.parseInt();
        
        groups[nr].animation = &flicker;
        groups[nr].selection = &flickerStrategy;
        groups[nr].counter = 0;
        groups[nr].waitFrames = 0;
        
        break;
        
      case roadAction:
        Serial.println("Select road group");   
        nr = Serial.parseInt();

        groups[nr].animation = &road;
        groups[nr].selection = &roadStrategy;
        groups[nr].counter = 0;
        groups[nr].waitFrames = 0;
        
        break;

        break;
        
      case hideAction:
        Serial.println("Select hide group");  
        nr = Serial.parseInt();

        Serial.println("Select hide percent");  
        groups[nr].hidePercent = Serial.parseInt();

        groups[nr].animation = &hide;
        groups[nr].selection = &hideStrategy;
        groups[nr].counter = 0;
        groups[nr].waitFrames = 0;

        break;

      default:
        break;
    }

  Serial.println("Done");
  while(Serial.available() > 0) Serial.read(); 
  Serial.setTimeout(50);
}

SerialAction intToAction(int value) {
  switch(value) {
    case 0:
      return showAction;

    case 1:
      return flickerAction;

    case 2:
      return roadAction;

    case 3:
      return hideAction;

    default:
      break;
  }

  return unknownAction;
}

