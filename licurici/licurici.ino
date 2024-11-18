#include <Adafruit_NeoPixel.h>
#include "types.h"

#include "group.h"
#include "groupAnimation.h"

#define HAS_AUDIO_SENSOR false
#define HAS_BUZZER true
#define HAS_DISTANCE_SENSOR false
#define ENABLE_RANDOM_COLOR true
#define HAS_WIFI true

#if HAS_DISTANCE_SENSOR
  #include <Ultrasonic.h>
#endif

#if HAS_WIFI
  #include "wifi.h"
#endif

#if HAS_BUZZER 
  #include "sound.h"
#endif

Adafruit_NeoPixel strip = Adafruit_NeoPixel(100, 2, NEO_RGB + NEO_KHZ800);

LedGroup groups[TOTAL_GROUPS];

#if HAS_DISTANCE_SENSOR
  Ultrasonic distanceSensor(7, 8);
  
  long oldDistanceTime = 0;
  
  int maxDistance = 250;
  int distanceValue = 0;
#endif

#if HAS_AUDIO_SENSOR
  const int audioPin = A0;
  const int soundAverage = 340;
  
  int soundThreshold = 140;
  int soundValue;
  unsigned long maxAudioPeek = 40;
  
  unsigned long staminaEnd;
  unsigned long staminaMilliseconds = 60000;
  
  bool audioLoopEnabled = false;
#endif

unsigned long hideTimestamp = 0;
long oldTime = 0;
long hilightFlickerTimeout = 0;

#if ENABLE_RANDOM_COLOR
  long lastRandomTime = 0;
#endif

void setup() {
  #if HAS_BUZZER 
    onSound();
  #endif

  while (!Serial);

  delay(2000);

  Serial.begin(9600);

  #if HAS_BUZZER 
    initSound();
  #endif

  #if HAS_AUDIO_SENSOR
    randomSeed(analogRead(audioPin));
  #endif
  
  randomColor();

  #if ENABLE_RANDOM_COLOR
    lastRandomTime = 0;
  #endif

  #if HAS_WIFI
    setup_WiFi();
    connectedSound();
  #endif

  strip.begin();
  strip.show();

  Serial.println("Setup strips");

  for (int i = 0; i < TOTAL_GROUPS; i++)
  {
    groups[i].setup(&strip, i * 50, 50);
  }

  resetAnimations();

  Serial.println("Start loop");
  #if HAS_BUZZER 
  readySound();
  #endif
}

void serialPrintStr(const char message[]) {
  Serial.print(message);
}

void serialPrintInt(int value) {
  Serial.print(value);
}

int serialReadInt() {
  return Serial.parseInt();
}

void flushReport(StringOut printStr, IntOut printInt) {
  auto currentColor = getCurrentColor();

  printStr("BEGIN REPORT\n");

  #if HAS_AUDIO_SENSOR
    printStr("audio loop: `");
    printStr(audioLoopEnabled ? "on`" : "off`");
  #else
    printStr("audio loop: disabled (no audio sensor)");
  #endif
  printStr("\n");
  
  printStr("current color ");

  printInt(Red(currentColor));
  printStr(" ");
  printInt(Green(currentColor));
  printStr(" ");
  printInt(Blue(currentColor));
  printStr(" = ");
  printInt(getCurrentColor());
  printStr("\n");
  printStr("");
  printStr("\n");

  #if ENABLE_RANDOM_COLOR
    printStr("next color in ");
    printInt(1080000 - millis() - lastRandomTime);
    printStr(" milliseconds");
    printStr("\n\n");
  #else
    printStr("RANDOM COLOR DISABLED");
    printStr("\n");
  #endif

  #if HAS_DISTANCE_SENSOR
    printStr("distance sensor cm");
    printInt(distanceValue);
    printStr("\n");
  #endif
  
  #if HAS_AUDIO_SENSOR
    printStr("audio sensor ");
    printInt(soundValue);
    printStr("\n");

    printStr("audio threshold ");
    printInt(soundThreshold);
    printStr("\n");
    printStr("audio stamina: ");

    if (isStamina()) {
      printStr("enabled");
      printStr("\n");

      printStr("Stamina left: ");
      printInt(staminaEnd - millis());
      printStr(" milliseconds");
      printStr("\n");
    } else {
      printStr("disabled\n");
    }
  #endif

  printStr("\n");

  printStr("groups ");
  printInt(TOTAL_GROUPS);
  printStr("\n");

  printStr("leds ");
  printStr(strip.numPixels());
  printStr("\n\n");

  for (int i = 0; i < TOTAL_GROUPS; i++) {
    printStr("*Group ");
    printInt(i);
    printStr("*");
    printStr("\n");

    printStr(" leds ");
    printInt(groups[i].length);
    printStr("\n");

    printStr(" start at ");
    printInt(groups[i].start);
    printStr("\n");

    printStr(" visible ");
    printInt(100 - groups[i].percentHidden());
    printStr("%");
    printStr("\n");

    printStr(" animation ");
    printStr(stringAnimation(&groups[i]));
    printStr("\n\n");
  }

  printStr("END REPORT\n");
}

void performAction(SerialAction action, StringOut printStr, IntOut printInt, IntIn readInt) {
  int nr;
  byte red;
  byte green;
  byte blue;
  Color currentColor;

  switch (action) {
    case showAction:
      printStr("Select show group\n");
      nr = readInt();

      groups[nr].animation = &show;
      groups[nr].selection = &showStrategy;
      groups[nr].counter = 0;
      groups[nr].waitFrames = 0;

      break;

    case flickerAction:
      printStr("Select flicker group\n");
      nr = readInt();

      groups[nr].animation = &flicker;
      groups[nr].selection = &flickerStrategy;
      groups[nr].counter = 0;
      groups[nr].waitFrames = 0;

      break;

    case hideAction:
      printStr("Select hide group\n");
      nr = readInt();

      printStr("Select hide percent\n");
      groups[nr].hidePercent = readInt();

      groups[nr].animation = &hide;
      groups[nr].selection = &hideStrategy;
      groups[nr].counter = 0;
      groups[nr].waitFrames = 0;

      break;
      
    case hideAllGroupsAction:
      nr = readInt();

      for(int i = 0; i<TOTAL_GROUPS; i++) {
        hideGroup(i, nr);
      }

      break;

    case happyAction:
      printStr("Select happy group\n");
      nr = readInt();

      groups[nr].animation = &happy;
      groups[nr].selection = &happyStrategy;
      groups[nr].counter = 0;
      groups[nr].waitFrames = 0;

      break;

    case allHappyAction:
      for (int i = 0; i < TOTAL_GROUPS; i++) {
        groups[i].animation = &happy;
        groups[i].selection = &happyStrategy;
        groups[i].counter = 0;
        groups[i].waitFrames = 0;
      }

      #if HAS_AUDIO_SENSOR
        enableStamina();
      #endif
      
      break;

    case staminaAction:
      #if HAS_AUDIO_SENSOR
        enableStamina();
      #endif

      break;

    case colorAction:
      printStr("red\n");
      red = readInt();

      printStr("green\n");
      green = readInt();

      printStr("blue\n");
      blue = readInt();

      setCurrentColor(createColor(red, green, blue));
      
      #if ENABLE_RANDOM_COLOR
        lastRandomTime = millis();
      #endif

      break;

    case colorRawAction:
      setCurrentColor(readInt());
      
      #if ENABLE_RANDOM_COLOR
        lastRandomTime = millis();
      #endif

      break;

    case reportAction:
      flushReport(&serialPrintStr, &serialPrintInt);
      break;

    case audioThresholdAction:
      Serial.println("Enter the new audio threshold");
      #if HAS_AUDIO_SENSOR
        soundThreshold = readInt();
      #else
        readInt();
      #endif
      
      break;

    case queryAudio:
      printStr("audio-level:");
      #if HAS_AUDIO_SENSOR
        printInt(soundValue);
      #else
        printInt(0);
      #endif
      printStr("\n");
      break;

    case distanceMeasurementAction:
      printStr("distance to object in cm: ");
      #if HAS_DISTANCE_SENSOR
        printInt(distanceValue);
      #else 
        printInt(0);
      #endif
      printStr("\n");
      break;

    case setHilightFlicker:
      hilightFlickerTimeout = millis();
      swapCurrentColor();

      for (int i = 0; i < TOTAL_GROUPS; i++) {
        groups[i].animation = &hilightFlicker;
        groups[i].selection = &hilightFlickerStrategy;
        groups[i].counter = 0;
        groups[i].waitFrames = 0;
      }

    default:
      break;
  }

  printStr("Done\n");
}

void resetAnimations() {
  Serial.println("Reset animations");

  groups[0].animation = &happy;
  groups[0].selection = &happyStrategy;

  groups[1].animation = &happy;
  groups[1].selection = &happyStrategy;

  groups[2].animation = &happy;
  groups[2].selection = &happyStrategy;

  groups[3].animation = &happy;
  groups[3].selection = &happyStrategy;

  for (int i = 0; i < TOTAL_GROUPS; i++)
  {
    groups[i].counter = 0;
    groups[i].waitFrames = 0;
    groups[i].hidePercent = 0;
  }
}

void hideAll() {
  for (int i = 0; i < TOTAL_GROUPS; i++)
  {
    hideGroup(i, 100);
  }
}

void hideGroup(int i, int percent) {
  if (groups[i].isAnimation(&hide)) {
    return;
  }

  int finalPercent = min(groups[i].percentHidden() + percent, 100);

  Serial.print("Hiding ");
  Serial.print(finalPercent);
  Serial.println("% leds");

  groups[i].animation = &hide;
  groups[i].selection = &hideStrategy;
  groups[i].counter = 0;
  groups[i].waitFrames = 0;
  groups[i].hidePercent = finalPercent;
  groups[i].animationDone = false;
}

void updateHidePercentGroup(int i) {
  if (groups[i].isAnimation(&hide) && groups[i].animationDone)
  {
    Serial.print("Show group ");
    Serial.println(i);
    
    groups[i].hidePercent = 0;
    groups[i].counter = 0;

    #if HAS_AUDIO_SENSOR
        groups[i].animation = isStamina() ? &flicker : &show;
        groups[i].selection = isStamina() ? &flickerStrategy : &showStrategy;
    #else
        groups[i].animation = &show;
        groups[i].selection = &showStrategy;
    #endif
  }
}


#if HAS_AUDIO_SENSOR
  void audioLoop() {
    audioLoopEnabled = true;
  
    int readValue = analogRead(audioPin);
    soundValue = abs(readValue - soundAverage);
  
    int localThreshold = isStamina() ? soundThreshold * 2 : soundThreshold;
  
    bool shouldHide = false;
  
    if (soundValue > localThreshold) {
      if (hideTimestamp == 0) {
        shouldHide = true;
      }
  
      hideTimestamp = millis();
    } else if (millis() - hideTimestamp > maxAudioPeek) {
      hideTimestamp  = 0;
    }
  
    if (shouldHide) {
  
      randomSeed(readValue);
  
      Serial.print("SoundDetected ");
      Serial.println(soundValue - localThreshold);
  
      int percent = min(100, (soundValue - localThreshold) / 4);
  
      Serial.print("hide:");
      Serial.println(percent);
  
      for (int i = 0; i < TOTAL_GROUPS; i++)
      {
        hideGroup(i, percent);
      }
    } else {
      for (int i = 0; i < TOTAL_GROUPS; i++)
      {
        updateHidePercentGroup(i);
      }
    }
  }

  boolean isStamina() {
    return millis() < staminaEnd;
  }
  
  void enableStamina() {
    staminaEnd = millis() + staminaMilliseconds;
  }
#endif

void loop() {
  if (millis() - oldTime >= 20) {
    for (int i = 0; i < TOTAL_GROUPS; i++) {
      groups[i].animate();
    }

    strip.show();
    oldTime = millis();
  }

  #if HAS_AUDIO_SENSOR
    audioLoop(); 
  #else
    for (int i = 0; i < TOTAL_GROUPS; i++)
    {
      updateHidePercentGroup(i);
    }
  #endif

  #if ENABLE_RANDOM_COLOR
    if (millis() - lastRandomTime >= 1080000) {
      lastRandomTime = millis();
      randomColor();
    }
  #endif

  #if HAS_DISTANCE_SENSOR
    if (groups[0].isAnimation(&show) || groups[0].isAnimation(&flicker)) {
      if (millis() - oldDistanceTime >= 100) {
        oldDistanceTime = millis();
          
        int value = distanceSensor.distanceRead();

        if(value > 0) {
          if(value > maxDistance) {
            maxDistance = value;
          }
    
          if(value < maxDistance - 10) {
            distanceValue = value;
          }
        }
      }
    }
  #endif

  #if HAS_WIFI
    loop_WiFi(*flushReport, *performAction);
  #endif
  
  bool flickerTimeout = millis() - hilightFlickerTimeout > 20000; // 20 seconds for the hilight flicker
  
  for (int i = 0; i < TOTAL_GROUPS; i++) {
    if (flickerTimeout && groups[i].isAnimation(&hilightFlicker)) {
      Serial.print("Hilight flicker timeout for group");
      Serial.println(i);
      groups[i].animation = &flicker;
      groups[i].selection = &flickerStrategy;
      groups[i].counter = 0;
      groups[i].waitFrames = 0;
    }
  }

  SerialAction action = unknownAction;

  if (Serial.available() > 0) {
    Serial.println("\n\nparse command");
   
    action = intToAction(Serial.parseInt());

    if (action == unknownAction) {
      Serial.println("Unknown command");
    }

    if (action != unknownAction) {
      Serial.setTimeout(5000);
      performAction(action, serialPrintStr, serialPrintInt, serialReadInt);

      while (Serial.available() > 0) Serial.read();
      Serial.setTimeout(10);
      action = unknownAction;
    }
  }
}

SerialAction intToAction(int value) {
  switch (value) {
    case 0:
      return showAction;

    case 1:
      return happyAction;

    case 2:
      return flickerAction;

    case 3:
      return roadAction;

    case 4:
      return hideAction;

    case 5:
      return colorAction;

    case 6:
      return reportAction;

    case 7:
      return allHappyAction;

    case 8:
      return staminaAction;

    case 9:
      return audioThresholdAction;

    case 11:
      return queryAudio;

    case 12:
      return distanceMeasurementAction;

    case 13:
      return setHilightFlicker;

    case 14:
      return hideAllGroupsAction;

    case 15:
      return colorRawAction;

    default:
      break;
  }

  return unknownAction;
}


