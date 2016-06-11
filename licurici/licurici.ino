#include <Adafruit_NeoPixel.h>

#include <Adafruit_WS2801.h>

//#include <Adafruit_WS2801.h>

#include "group.h"
#include "groupAnimation.h"

const uint8_t dataPin  = 2;    // Yellow wire on Adafruit Pixels
const uint8_t clockPin = 3;    // Green wire on Adafruit Pixels

Adafruit_NeoPixel strip = Adafruit_NeoPixel(100, 2, NEO_RGB + NEO_KHZ800);

#define TOTAL_GROUPS 2

LedGroup groups[TOTAL_GROUPS];
 
enum SerialAction {
  showAction,
  flickerAction,
  roadAction,
  hideAction,
  happyAction,
  colorAction,
  reportAction,
  unknownAction
};

const int audioPin = A0;
const int soundAverage = 340;
const int soundThreshold = 40;
int soundValue;

const int LightPin = 3;
volatile unsigned long LightCnt = 0;
unsigned long oldLightCnt = 0;
unsigned long LightT = 0;
unsigned long LightLast;
unsigned long lightValue;

unsigned long lastHideUpdateTime;

const unsigned long lightThreshold = 160;

long oldTime = 0;

void irq1()
{
  LightCnt++;
}

void setup() {  
  delay(2000);

  Serial.begin(9600);

  pinMode(LightPin, INPUT);
  digitalWrite(LightPin, HIGH);
  attachInterrupt(0, irq1, RISING);

  lastHideUpdateTime = 0;

  randomSeed(analogRead(audioPin));

  strip.begin();
  strip.show();
  
  Serial.println("Setup strips"); 
  groups[0].setup(&strip, 1, 50);
  groups[0].animation = &flicker;
  groups[0].selection = &flickerStrategy;

  groups[1].setup(&strip, 50, 50);
  groups[1].animation = &flicker;
  groups[1].selection = &flickerStrategy;
/*
  groups[2].setup(&strip, 100, 50);
  groups[2].animation = &flicker;
  groups[2].selection = &flickerStrategy;

  groups[3].setup(&strip, 150, 50);
  groups[3].animation = &flicker;
  groups[3].selection = &flickerStrategy;


  
/*
  groups[1].setup(&strip, 10, 15);
  groups[1].animation = &flicker;
  groups[1].selection = &flickerStrategy;*/

  
  Serial.println("Start loop"); 
  
  //colorWipe(strip.Color(255, 0, 0), 50); // Red
  
}

void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}


void hideGroup(int i, int percent) {
  if(groups[i].isAnimation(&hide) || groups[i].isAnimation(&road)) {
    return;
  }

  groups[i].animation = &hide;
  groups[i].selection = &hideStrategy;
  groups[i].counter = 0;
  groups[i].waitFrames = 0;
  groups[i].hidePercent = min(groups[i].hidePercent + percent, 100);
  groups[i].animationDone = false;
}

void updateHidePercentGroup(int i) {
  if(groups[i].isAnimation(&hide) && groups[i].animationDone) 
  {
    Serial.print("Show group ");
    Serial.println(i);

    groups[i].animation = &show;
    groups[i].selection = &showStrategy;
    groups[i].counter = 0;
  } else if(!groups[i].isAnimation(&show) && groups[i].waitFrames == 0) {
    int diff = millis() - lastHideUpdateTime;

    if(diff > 1000) {
      groups[i].hidePercent = max(1, groups[i].hidePercent - 1);
      lastHideUpdateTime = millis();
    }
  }
}

void audioLoop() {
  int readValue = analogRead(audioPin);
  soundValue = abs(readValue- soundAverage);

  if(soundValue > soundThreshold) {
    randomSeed(readValue);

    Serial.print("SoundDetected ");
    Serial.println(soundValue - soundThreshold);

    int percent = min(100, soundValue - soundThreshold);

    Serial.print("Hiding ");
    Serial.print(percent);
    Serial.println("% leds");

    for(int i=0; i<TOTAL_GROUPS; i++)
    {
      hideGroup(i, percent);
    }
  } else {
    for(int i=0; i<TOTAL_GROUPS; i++)
    {
      updateHidePercentGroup(i);
    }
  }
}

void lightLoop() {
  if (millis() - LightLast > 1000)
  {
    LightLast = millis();
    LightT = LightCnt;
    unsigned long hz = LightT - oldLightCnt;
    oldLightCnt = LightT;
    lightValue = (hz+50)/100;
  }
}

void loop() {

  //audioLoop();
  //lightLoop();

  if(millis() - oldTime >= 20) {
    
    for(int i=0; i<TOTAL_GROUPS; i++) {
      if(lightThreshold >= lightValue || groups[i].isAnimation(&hide) ) {
        groups[i].animate();
      }
    }
    
    strip.show();
    
    oldTime = millis();
  }
  
  SerialAction action = unknownAction;

  if(Serial.available() > 0) {
    Serial.println("\n\nparse command");  
    action = intToAction(Serial.parseInt());

    if(action == unknownAction) {
      Serial.println("Unknown command");  
    }
 
    if(action != unknownAction) {
      performAction(action);
      action = unknownAction;
    }
  }
}

void performAction(SerialAction action) {
    int nr;
    byte red;
    byte green;
    byte blue;
    Color currentColor;
    
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

      case happyAction:
        Serial.println("Select happy group");  
        nr = Serial.parseInt();

        groups[nr].animation = &happy;
        groups[nr].selection = &happyStrategy;
        groups[nr].counter = 0;
        groups[nr].waitFrames = 0;

        break;
      
      case colorAction:
        Serial.println("red");  
        red = Serial.parseInt();

        Serial.println("green");  
        green = Serial.parseInt();
        
        Serial.println("blue");  
        blue = Serial.parseInt();
        
        setCurrentColor(createColor(red, green, blue));

        break;

      case reportAction: 
        currentColor = getCurrentColor();

        Serial.println("BEGIN REPORT");

        Serial.print("current color ");
        Serial.print(Red(currentColor));
        Serial.print(" ");
        Serial.print(Green(currentColor));
        Serial.print(" ");
        Serial.println(Blue(currentColor));
        Serial.println("");
  
        Serial.print("light sensor ");
        Serial.print(lightValue);
        Serial.println(" mW/m2");
        
        Serial.print("light threshold ");
        Serial.println(lightThreshold);
        Serial.println("");

        Serial.print("audio sensor ");
        Serial.println(soundValue);
        
        Serial.print("audio threshold ");
        Serial.println(soundThreshold);
        Serial.println("");
        
        Serial.print("groups ");
        Serial.println(TOTAL_GROUPS);

        Serial.print("leds ");
        Serial.println(strip.numPixels());
        Serial.println("");

        for(int i=0; i<TOTAL_GROUPS; i++) {
          Serial.print("Group ");
          Serial.println(i);

          Serial.print(" leds ");
          Serial.println(groups[i].length);
          
          Serial.print(" start at ");
          Serial.println(groups[i].start);

          Serial.print(" visible ");
          Serial.print(100 - groups[i].percentHidden());
          Serial.println("%");

          Serial.print(" animation ");
          Serial.println(stringAnimation(&groups[i]));
          Serial.println("");
        }
        
        Serial.println("END REPORT");
        
        
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
      return happyAction;

    case 4:
      return hideAction;

    case 5:
      return colorAction;

    case 6:
      return reportAction;

    default:
      break;
  }

  return unknownAction;
}


