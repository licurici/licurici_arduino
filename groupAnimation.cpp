#include "groupAnimation.h"


Color currentColor = createColor(0, 10, 5);

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
  group->selectionLen = min(10, group->length - 2);

  for(int i=0; i<group->selectionLen; i++) {
    group->state[i] = 4;
    
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
      group->targetColors[0] = currentColor;
      group->state[0]++;
      break;

    case 1:
      checkState(group, 0, 0);
      break;

    case 2:
      group->targetColors[0] = createColor(0,0,0);
      group->targetColors[1] = currentColor;
      group->state[0]++;
      break;
    
    case 3:
      checkState(group, 1, 0);
      break;
      
    case 4:
      group->targetColors[1] = createColor(0,0,0);
      group->targetColors[2] = currentColor;
      group->state[0]++;
      break;
    
    case 5:
      checkState(group, 2, 0);
      break;

    case 6:
      group->targetColors[2] = createColor(0,0,0);
      group->state[0]++;
      break;

    case 7:
      checkState(group, 2, 0);
      break;
      
    default:
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
    
  group->selected[i] = selected;
}

void hide(LedGroup* group) {
  for(int i=0;i<group->selectionLen; i++) {
    switch(group->state[i]) {
        case 0:
          group->targetColors[i] = 0;
          group->state[i]++;
          break;
  
        case 2:
          selectHideLed(group, i);
          group->state[i] = 0;
          break;
          
        default:
          checkState(group, i, i);
     }
   }
}

void hideStrategy(LedGroup* group) {
  group->selectionLen = 10;
  group->animationDone = false;

  for(int i=0;i<group->selectionLen; i++) {
    group->state[i] = 2;
    
    group->increment[i][0] = random(1, 5);
    group->increment[i][1] = random(1, 5);
    group->increment[i][2] = random(1, 5);
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
    
  group->selected[i] = selected;
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
    switch(group->state[i]) {
        case 0:
          showLed(group, i);
          group->state[i]++;
          break;
  
        case 2:
          selectShowLed(group, i);
          group->state[i] = 0;
          break;
          
        default:
          checkState(group, i, i);
     }
   }
}

void showStrategy(LedGroup* group) {
  group->selectionLen = 3;

  for(int i=0;i<10; i++) {
    group->increment[i][0] = 1;
    group->increment[i][1] = 1;
    group->increment[i][2] = 1;

    group->state[i] = 2;
  }
}

