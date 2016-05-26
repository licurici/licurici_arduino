#include "groupAnimation.h"


Color currentColor = createColor(10, 0, 10);

void checkState(LedGroup* group, int i, int state) {
  if(group->reachedTarget(i)) {
    group->state[state]++;
  }
}

void flickerSelectColor(LedGroup* group, int i) {
  byte r = Red(currentColor) * random(2, 5);
  byte g = Green(currentColor) * random(2, 5);
  byte b = Blue(currentColor) * random(2, 5);
  
  group->targetColors[i] = createColor(r,g,b);
}

void selectFlickerLeds(LedGroup* group, int i) {
  int selected = random(0, group->length);

  while(group->isSelected(selected)) {
    selected = random(0, group->length);
  }
    
  group->selected[i] = selected;
}

void flicker(LedGroup* group) {

  for(int i=0; i<group->selectionLen; i++) {

    switch(group->state[i]) {
      case 0:
        flickerSelectColor(group, i);
        group->state[i]++;
        break;

      case 2:
        group->targetColors[i] = currentColor;
        group->state[i]++;
        break;

      case 4:
        selectFlickerLeds(group, i);
        group->state[i] = 0;
        break;
        
      default:
        checkState(group, i, i);
    }
  }
}

void flickerStrategy(LedGroup* group) {
  group->selectionLen = 10;

  for(int i=0; i<group->selectionLen; i++) {
    group->state[i] = 2;
    
    group->increment[i][0] = 1;
    group->increment[i][1] = 1;
    group->increment[i][2] = 1;
  }
}

void selectRoadLeds(LedGroup* group) {
  group->selectionLen = 2;
  group->selected[0] = group->selected[1];
  group->selected[1]++;

  if(group->selected[0] >= group->length) {
    group->selected[0] = 0;
    group->selected[1] = 1;
  }
}

void nextRoad(LedGroup* group) {

  if(group->selected[2] + 3 < group->length) {
    group->selected[0] += 3;
    group->selected[1] += 3;
    group->selected[2] += 3;  
  } else {
    group->selected[0] = 0;
    group->selected[1] = 1;
    group->selected[2] = 2;  
  }
}

void road(LedGroup* group) {

  switch(group->state[0]) {
    case 0:
      Serial.println(group->state[0]);
      group->targetColors[0] = currentColor;
      group->state[0]++;
      break;

    case 1:
      checkState(group, 0, 0);
      break;

    case 2:
      Serial.println(group->state[0]);
      group->targetColors[0] = createColor(0,0,0);
      group->targetColors[1] = currentColor;
      group->state[0]++;
      break;
    
    case 3:
      checkState(group, 1, 0);
      break;
      
    case 4:
      Serial.println(group->state[0]);
      group->targetColors[1] = createColor(0,0,0);
      group->targetColors[2] = currentColor;
      group->state[0]++;
      break;
    
    case 5:
      checkState(group, 2, 0);
      break;

    case 6:
      Serial.println(group->state[0]);
      group->targetColors[2] = createColor(0,0,0);
      group->state[0]++;
      break;

    case 7:
      checkState(group, 2, 0);
      break;
      
    default:
      Serial.println(group->state[0]);
      group->waitFrames = 50;
      nextRoad(group);
      group->state[0] = 0;
  }
}

void roadStrategy(LedGroup* group) {
  group->selectionLen = 3;
  group->selected[0] = 0;
  group->selected[1] = 1;
  group->selected[2] = 2;
  group->state[0] = 0;

  for(int i=0; i<=2; i++) {
    group->increment[i][0] = 3;
    group->increment[i][1] = 3;
    group->increment[i][2] = 3;
  }
}

void randomStrategy(LedGroup* group) {
  group->selectionLen = 10;

  for(int i=0; i<10; i++) {
    group->state[i] = 1;
    group->increment[i][0] = 1;
    group->increment[i][1] = 1;
    group->increment[i][2] = 1;
    group->selected[i] = random(0, group->length);
  }
}


