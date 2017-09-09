#include <Adafruit_NeoPixel.h>

#include <Adafruit_WS2801.h>

#include "group.h"
#include "groupAnimation.h"

const uint8_t dataPin  = 2;    // Yellow wire on Adafruit Pixels
const uint8_t clockPin = 3;    // Green wire on Adafruit Pixels

Adafruit_NeoPixel strip = Adafruit_NeoPixel(200, 2, NEO_RGB + NEO_KHZ800);

#define TOTAL_GROUPS 4

LedGroup groups[TOTAL_GROUPS];

enum SerialAction {
  showAction,
  flickerAction,
  roadAction,
  hideAction,
  happyAction,
  colorAction,
  reportAction,
  allHappyAction,
  staminaAction,
  audioThresholdAction,
  lightThresholdAction,
  queryAudio,
  unknownAction
};

const int audioPin = A0;
const int soundAverage = 340;

int soundThreshold = 70;
int soundValue;
unsigned long audioTimestamp;
unsigned long maxAudioPeek = 40;

const int LightPin = 3;
volatile unsigned long LightCnt = 0;
unsigned long oldLightCnt = 0;
unsigned long LightT = 0;
unsigned long LightLast;
unsigned long lightValue;

unsigned long staminaEnd;
unsigned long staminaMilliseconds = 60000;

unsigned long lightThreshold = 90;

long oldTime = 0;
long lastRandomTime = 0;

bool audioLoopEnabled = false;
bool lightLoopEnabled = false;
bool isDark = false;

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

  randomSeed(analogRead(audioPin));
  randomColor();
  lastRandomTime = 0;

  strip.begin();
  strip.show();

  Serial.println("Setup strips");

  groups[0].setup(&strip, 0, 50);
  groups[1].setup(&strip, 50, 48);
  groups[2].setup(&strip, 98, 52);
  groups[3].setup(&strip, 150, 50);

  resetAnimations();

  Serial.println("Start loop");
}

void resetAnimations() {
  Serial.println("Reset animations");

  groups[0].animation = &happy;
  groups[0].selection = &happyStrategy;

  groups[1].animation = &road;
  groups[1].selection = &roadStrategy;

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
  if (groups[i].isAnimation(&hide) || groups[i].isAnimation(&road)) {
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

    groups[i].animation = isStamina() ? &flicker : &show;
    groups[i].selection = isStamina() ? &flickerStrategy : &showStrategy;
    groups[i].counter = 0;
  }
}

void audioLoop() {
  audioLoopEnabled = true;

  int readValue = analogRead(audioPin);
  soundValue = abs(readValue - soundAverage);

  int localThreshold = isStamina() ? soundThreshold * 2 : soundThreshold;

  bool shouldHide = false;

  if (soundValue > localThreshold) {
    if (audioTimestamp == 0) {
      shouldHide = true;
    }

    audioTimestamp = millis();
  } else if (millis() - audioTimestamp > maxAudioPeek) {
    audioTimestamp = 0;
  }

  if (shouldHide) {

    randomSeed(readValue);

    Serial.print("SoundDetected ");
    Serial.println(soundValue - localThreshold);

    int percent = min(100, (soundValue - localThreshold) / 4);

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

void lightLoop() {
  lightLoopEnabled = true;

  if (millis() - LightLast > 1000)
  {
    LightLast = millis();
    LightT = LightCnt;
    unsigned long hz = LightT - oldLightCnt;
    oldLightCnt = LightT;
    lightValue = (hz + 50) / 100;

    isDark = lightValue < lightThreshold;
  }
}

boolean isStamina() {
  return millis() < staminaEnd;
}

void enableStamina() {
  staminaEnd = millis() + staminaMilliseconds;
}

void loop() {
  lightLoop();

  if (millis() - oldTime >= 20) {
    for (int i = 0; i < TOTAL_GROUPS; i++) {
      if (isDark || groups[i].isAnimation(&hide)) {
        groups[i].animate();
      }
    }

    strip.show();

    oldTime = millis();
  }

  if (isDark) {
    audioLoop();
  } else {
    hideAll();
  }

  if (millis() - lastRandomTime >= 1080000) {
    lastRandomTime = millis();
    randomColor();
  }

  SerialAction action = unknownAction;

  if (Serial.available() > 0) {
    Serial.println("\n\nparse command");
    action = intToAction(Serial.parseInt());

    if (action == unknownAction) {
      Serial.println("Unknown command");
    }

    if (action != unknownAction) {
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

  switch (action) {
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

    case allHappyAction:
      for (int i = 0; i < TOTAL_GROUPS; i++) {
        if (!groups[i].isAnimation(&road)) {
          groups[i].animation = &happy;
          groups[i].selection = &happyStrategy;
          groups[i].counter = 0;
          groups[i].waitFrames = 0;
        }
      }

      enableStamina();
      break;

    case staminaAction:

      enableStamina();
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

      Serial.print("audio loop: `");
      Serial.println(audioLoopEnabled ? "on`" : "off`");

      Serial.print("light loop: `");
      Serial.println(lightLoopEnabled ? "on`" : "off`");
      Serial.println();

      Serial.print("isDark: ");
      Serial.println(isDark);
      Serial.println();

      Serial.print("current color ");

      Serial.print(Red(currentColor));
      Serial.print(" ");
      Serial.print(Green(currentColor));
      Serial.print(" ");
      Serial.print(Blue(currentColor));
      Serial.print(" = ");
      Serial.println(getCurrentColor());
      Serial.println("");
      
      Serial.print("next color in ");
      Serial.print(1080000 - millis() - lastRandomTime);
      Serial.println(" milliseconds");
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
      Serial.print("audio stamina: ");

      if (isStamina()) {
        Serial.println("enabled");

        Serial.print("Stamina left: ");
        Serial.print(staminaEnd - millis());
        Serial.println(" milliseconds");
      } else {
        Serial.println("disabled");
      }


      Serial.println("");

      Serial.print("groups ");
      Serial.println(TOTAL_GROUPS);

      Serial.print("leds ");
      Serial.println(strip.numPixels());
      Serial.println("");

      for (int i = 0; i < TOTAL_GROUPS; i++) {
        Serial.print("*Group ");
        Serial.print(i);
        Serial.println("*");

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

    case audioThresholdAction:
      Serial.println("Enter the new audio threshold");
      soundThreshold = Serial.parseInt();

      break;

    case lightThresholdAction:
      Serial.println("Enter the new light threshold");
      lightThreshold = Serial.parseInt();

      break;
    case queryAudio:
      Serial.print("audio-level:");
      Serial.print(soundValue);
      Serial.print("\n");
      break;
  
    default:
      break;
  }

  Serial.println("Done");
  while (Serial.available() > 0) Serial.read();
  Serial.setTimeout(50);
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

    case 10:
      return lightThresholdAction;

    case 11:
      return queryAudio;

    default:
      break;
  }

  return unknownAction;
}


