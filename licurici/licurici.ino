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
  colorAction,
  unknownAction
};

const int audioPin = A0;
const int soundAverage = 340;
const int soundThreshold = 40;


const int LightPin = 3;
volatile unsigned long LightCnt = 0;
unsigned long oldLightCnt = 0;
unsigned long LightT = 0;
unsigned long LightLast;
unsigned long lightValue;

const unsigned long lightThreshold = 100;


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

  strip.begin();
  
  // Update LED contents, to start they are all 'off'
  strip.show();
  
  Serial.println("Setup strips"); 
  groups[0].setup(&strip, 0, 50);
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
}

void audioLoop() {
  int readValue = analogRead(audioPin);
  int sensorValue = abs(readValue- soundAverage);

  if(sensorValue > soundThreshold) {
    randomSeed(readValue);

    Serial.print("SoundDetected ");
    Serial.println(sensorValue - soundThreshold);

    int percent = min(100, sensorValue - soundThreshold);

    Serial.print("Hiding ");
    Serial.print(percent);
    Serial.println("% leds");
    
    for(int i=0; i<TOTAL_GROUPS; i++)
    {
      groups[i].animation = &hide;
      groups[i].selection = &hideStrategy;
      groups[i].counter = 0;
      groups[i].waitFrames = 0;
      groups[i].hidePercent = min(groups[i].hidePercent + percent, 100);
      groups[i].animationDone = false;
    }
  }
  else {
    for(int i=0; i<TOTAL_GROUPS; i++)
    {
      if(groups[i].isAnimation(&hide) && groups[i].animationDone) 
      {
        Serial.print("Show group ");
        Serial.println(i);
    
        groups[i].animation = &show;
        groups[i].selection = &showStrategy;
        groups[i].counter = 0;
      }

      if(!groups[i].isAnimation(&show) && groups[i].waitFrames == 0) {
        groups[i].hidePercent = max(1, groups[i].hidePercent - 1);
      }
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

  audioLoop();
  lightLoop();

  if(lightThreshold > lightValue) {
    return;
  }
  
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
      
      case colorAction:
        Serial.println("red");  
        red = Serial.parseInt();

        Serial.println("green");  
        green = Serial.parseInt();
        
        Serial.println("blue");  
        blue = Serial.parseInt();
        
        setCurrentColor(createColor(red, green, blue));

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

    case 4:
      return colorAction;

    default:
      break;
  }

  return unknownAction;
}


