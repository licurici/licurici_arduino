#include "groupAnimation.h"

Color currentColor = createColor(100, 50, 50);
Color availableColors[7] = {856835, 857092, 660480, 398337, 985600, 656128, 657920};

void randomColor() {
  int index = random(0, 7);
  Serial.print("color:");
  Serial.println(availableColors[index]);
  currentColor = availableColors[index];
}

void setCurrentColor(Color color) {
  currentColor = color;
}

Color getCurrentColor() {
  return currentColor;
}

void updateState(LedGroup* group, int i, int state) {
  if(group->pixelHasTargetColor(i)) {
    group->pixelState[state]++;
  }
}

void flickerSelectColor(LedGroup* group, int i) {
  float proportion = random(2, 7);
  float fr = Red(currentColor);
  float fg = Green(currentColor);
  float fb = Blue(currentColor);

  byte r = fr / proportion;
  byte g = fg / proportion;
  byte b = fb / proportion;

  Color c = group->getColor(i);
  float cr = Red(c);
  float cg = Green(c);
  float cb = Blue(c);

  group->increment[i][0] = max(abs(cr - fr) / 10, 1);
  group->increment[i][1] = max(abs(cg - fg) / 10, 1);
  group->increment[i][2] = max(abs(cb - fb) / 10, 1);

  group->targetColors[i] = createColor(r,g,b);
}

void selectFlickerLeds(LedGroup* group, int i) {
  int selected = random(0, group->length);

  while(group->isPixelAnimated(selected)) {
    selected = random(0, group->length);
  }

  group->animatingPixel[i] = selected;
}

void setProportionalTargetColor(LedGroup* group, int i, float colorIntensity) {
  float fr = Red(currentColor);
  float fg = Green(currentColor);
  float fb = Blue(currentColor);
  
  byte r = (fr / 100.) * colorIntensity;
  byte g = (fg / 100.) * colorIntensity;
  byte b = (fb / 100.) * colorIntensity;

  group->targetColors[i] = createColor(r, g, b);
}

void baseFlicker(LedGroup* group, int intensity) {

  for(int i=0; i<group->selectionLen; i++) {

    switch(group->pixelState[i]) {
      case 0:
        flickerSelectColor(group, i);
        group->pixelState[i]++;
        break;

      case 2:
        setProportionalTargetColor(group, i, intensity);
        group->pixelState[i]++;
        break;

      case 4:
        selectFlickerLeds(group, i);
        group->pixelState[i] = 0;
        break;

      default:
        updateState(group, i, i);
    }
  }
}

void hilightFlicker(LedGroup* group) {
  baseFlicker(group, 400);
}

void flicker(LedGroup* group) {
  baseFlicker(group, 100);
}

void flickerStrategy(LedGroup* group) {
  group->selectionLen = min(10, group->length - 2);

  for(int i=0; i<group->selectionLen; i++) {
    group->pixelState[i] = 4;
  }
}

void selectRoadLeds(LedGroup* group) {
  group->selectionLen = 2;
  group->animatingPixel[0] = group->animatingPixel[1];
  group->animatingPixel[1]++;

  if(group->animatingPixel[0] >= group->length) {
    group->animatingPixel[0] = 0;
    group->animatingPixel[1] = 1;
  }
}

int nextUnselected(LedGroup* group) {
  int last = 0;

  for(int i=0;i<group->selectionLen; i++) {
    if(group->animatingPixel[i] > last) {
      last = group->animatingPixel[i];
    }
  }

  return last + 1;
}

int firstUnselected(LedGroup* group) {
  int first = 100;

  for(int i=0;i<group->selectionLen; i++) {
    if(group->animatingPixel[i] < first) {
      first = group->animatingPixel[i];
    }
  }

  return first - 1;
}

void happy(LedGroup* group) {
  byte r = Red(currentColor);
  byte g = Green(currentColor);
  byte b = Blue(currentColor);

  Color highColor1 = createColor(r/ 2,  g/2, b/2);
  Color highColor2 = createColor(250, 250, 250);

  for(int i=0;i<group->selectionLen; i++) {
  switch(group->pixelState[i]) {
    case 0:
      group->targetColors[i] = highColor1;
      group->pixelState[i]++;
      break;

    case 1:
      updateState(group, i, i);
      break;

    case 2:
      if(i == group->selectionLen - 1 && group->selectionLen < 10) {
        group->animatingPixel[group->selectionLen] = nextUnselected(group);
        group->pixelState[group->selectionLen] = 0;
        group->selectionLen++;
      }

      group->targetColors[i] = highColor2;
      group->pixelState[i]++;

      break;

    case 3:
      updateState(group, i, i);
      break;

    case 4:
      group->targetColors[i] = currentColor;
      group->pixelState[i]++;
      break;

    case 5:
      updateState(group, i, i);
      break;

    default:
      group->pixelState[i] = 0;
      group->animatingPixel[i] = nextUnselected(group);

      if(firstUnselected(group) >= group->length - 1) {
        group->animation = &flicker;
        group->selection = &flickerStrategy;
        group->counter = 0;
        group->waitFrames = 0;
      }
    }
  }
}

void happyStrategy(LedGroup* group) {
  group->selectionLen = 1;

  for(int i=0; i<10; i++) {
    group->increment[i][0] = 10;
    group->increment[i][1] = 10;
    group->increment[i][2] = 10;

    group->animatingPixel[i] = 0;
    group->pixelState[0] = 0;
  }
}

void selectHideLed(LedGroup* group, int i) {
  int selected = random(0, group->length);
  int index = 0;

  if(group->percentHidden() >= group->hidePercent) {
    group->waitFrames = 100;
    group->animationDone = true;
    return;
  }

  //select a random led
  while(group->isHidden(selected) || index == 10) {
    selected = random(0, group->length);
    index++;
  }

  //if we can't find a random led
  if(index == 10) {
    for(int i=0; i<group->length; i++) {
      if(!group->isHidden(i)) {
        selected = i;
        index = 0;
      }
    }
  }

  if(index == 10) {
    group->waitFrames = 100;
  }

  group->animatingPixel[i] = selected;
}

void hide(LedGroup* group) {
  for(int i=0;i<group->selectionLen; i++) {
    switch(group->pixelState[i]) {
        case 0:
          group->targetColors[i] = 0;
          group->pixelState[i]++;
          break;

        case 2:
          selectHideLed(group, i);
          group->pixelState[i] = 0;
          break;

        default:
          updateState(group, i, i);
     }
   }
}

void hideStrategy(LedGroup* group) {
  group->selectionLen = 10;
  group->animationDone = false;

  for(int i=0;i<group->selectionLen; i++) {
    group->pixelState[i] = 2;

    byte proportion = random(1, 5);

    group->increment[i][0] = Red(currentColor) / proportion;
    group->increment[i][1] = Green(currentColor) / proportion;
    group->increment[i][2] = Blue(currentColor) / proportion;
  }
}

void selectShowLed(LedGroup* group, int i) {
  int selected = random(0, group->length);
  int index = 0;

  if(group->percentHidden() == 0) {
    group->animation = flicker;
    group->selection = flickerStrategy;
    group->counter = 0;

    return;
  }

  //select a random led
  while(group->getColor(selected) == currentColor || index == 10) {
    selected = random(0, group->length);
    index++;
  }

  //if we can't find a random led
  if(index == 10) {
    for(int i=0; i<group->length; i++) {
      if(group->getColor(selected) != currentColor) {
        selected = i;
        index = 0;
      }
    }
  }

  if(index == 10) {
    group->waitFrames = 100;
  }

  group->animatingPixel[i] = selected;
}

void showLed(LedGroup* group, int i) {
  int index = random(1, 3);

  byte r = min(Red(currentColor) + index, Red(currentColor));
  byte g = min(Green(currentColor) + index, Green(currentColor));
  byte b = min(Blue(currentColor) + index, Blue(currentColor));

  group->waitFrames = 100;
  group->targetColors[i] = createColor(r,g,b);
}

void show(LedGroup* group) {
  for(int i=0;i<group->selectionLen; i++) {
    switch(group->pixelState[i]) {
        case 0:
          showLed(group, i);
          group->pixelState[i]++;
          break;

        case 2:
          selectShowLed(group, i);
          group->pixelState[i] = 0;
          break;

        default:
          updateState(group, i, i);
     }
   }
}

void showStrategy(LedGroup* group) {
  group->selectionLen = 3;

  for(int i=0;i<10; i++) {
    group->increment[i][0] = 1;
    group->increment[i][1] = 1;
    group->increment[i][2] = 1;

    group->pixelState[i] = 2;
  }
}

const char* stringAnimation(LedGroup* group) {

  if(group->animation == hide) {
    return "hide";
  }

  if(group->animation == show) {
    return "show";
  }

  if(group->animation == flicker) {
    return "flicker";
  }

  if(group->animation == happy) {
    return "happy";
  }

  return "unknown";
}
