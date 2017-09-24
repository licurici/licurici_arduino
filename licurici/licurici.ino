#include <Ultrasonic.h>
#include <Adafruit_NeoPixel.h>

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
  hideAllGroupsAction,
  happyAction,
  colorAction,
  colorRawAction,
  reportAction,
  allHappyAction,
  staminaAction,
  audioThresholdAction,
  queryAudio,
  distanceMeasurementAction,
  setHilightFlicker,
  unknownAction
};

Ultrasonic distanceSensor(7);

const int audioPin = A0;
const int soundAverage = 340;

int soundThreshold = 70;
int soundValue;
unsigned long audioTimestamp;
unsigned long maxAudioPeek = 40;

unsigned long staminaEnd;
unsigned long staminaMilliseconds = 60000;

long oldTime = 0;
long lastRandomTime = 0;
long oldDistanceTime = 0;

bool audioLoopEnabled = false;
bool isDark = false;

void setup() {
  delay(2000);

  Serial.begin(9600);

  randomSeed(analogRead(audioPin));
  randomColor();
  lastRandomTime = 0;

  strip.begin();
  strip.show();

  Serial.println("Setup strips");

  groups[0].setup(&strip, 0, 50);
  groups[1].setup(&strip, 50, 50);
  groups[2].setup(&strip, 100, 50);
  groups[3].setup(&strip, 150, 50);

  resetAnimations();

  Serial.println("Start loop");
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

byte intensityFromDistance(long distance) {
  if(distance < 100) {
    return distance;
  }

  return min(100, max(0, distance - 100));
}

void loop() {
  if (millis() - oldTime >= 20) {
    for (int i = 0; i < TOTAL_GROUPS; i++) {
      groups[i].animate();
    }

    strip.show();
    oldTime = millis();
  }

  audioLoop();

  if (millis() - lastRandomTime >= 1080000) {
    lastRandomTime = millis();
    randomColor();
  }

  if (groups[0].isAnimation(&show) || groups[0].isAnimation(&flicker)) {
    if (millis() - oldDistanceTime >= 500) {
      //distanceSensor.MeasureInCentimeters();
      //setFlickerIntensity(intensityFromDistance(distanceSensor.RangeInCentimeters));
      oldDistanceTime = millis();
      //Serial.print(distanceSensor.RangeInCentimeters);
      //Serial.print(" ");
      //Serial.println(intensityFromDistance(distanceSensor.RangeInCentimeters));
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
    case hideAllGroupsAction:
      nr = Serial.parseInt();

      for(int i = 0; i<TOTAL_GROUPS; i++) {
        if(groups[i].hidePercent < nr) {
          continue;
        }

        groups[i].hidePercent = nr;

        groups[i].animation = &hide;
        groups[i].selection = &hideStrategy;
        groups[i].counter = 0;
        groups[i].waitFrames = 0;
      }

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
        groups[i].animation = &happy;
        groups[i].selection = &happyStrategy;
        groups[i].counter = 0;
        groups[i].waitFrames = 0;
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

    case colorRawAction:
      setCurrentColor(Serial.parseInt());
      break;

    case reportAction:
      currentColor = getCurrentColor();

      Serial.println("BEGIN REPORT");

      Serial.print("audio loop: `");
      Serial.println(audioLoopEnabled ? "on`" : "off`");

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

      Serial.print("distance sensor cm");
      Serial.println(0);//distanceSensor.RangeInCentimeters);

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

    case queryAudio:
      Serial.print("audio-level:");
      Serial.print(soundValue);
      Serial.print("\n");
      break;

    case distanceMeasurementAction:
      Serial.print("distance to object in cm: ");
      Serial.print(0);//distanceSensor.RangeInCentimeters);
      Serial.print("\n");
      break;

    case setHilightFlicker:
      for (int i = 0; i < TOTAL_GROUPS; i++) {
        groups[i].animation = &hilightFlicker;
        groups[i].selection = &flickerStrategy;
        groups[i].counter = 0;
        groups[i].waitFrames = 0;
      }

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



