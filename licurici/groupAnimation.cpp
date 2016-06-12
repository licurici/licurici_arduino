#include "groupAnimation.h"

Color currentColor = createColor(0, 10, 5);
Color availableColors[5] = {2565, 655370, 1290, 655360, 2560};

void randomColor() {
  int index = random(0, 4);
  currentColor = availableColors[index];
}

void setCurrentColor(Color color) {
  currentColor = color;
}

Color getCurrentColor() {
  return currentColor;
}

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

void roadAnimation(LedGroup* group, int wait, Color color1, Color color2) {
      
    switch(group->state[0]) {
    case 0:
      group->targetColors[0] = color1;
      group->state[0]++;
      break;

    case 1:
      checkState(group, 0, 0);
      break;

    case 2:
      group->targetColors[0] = color2;
      group->targetColors[1] = color1;
      group->state[0]++;
      break;
    
    case 3:
      checkState(group, 1, 0);
      break;
      
    case 4:
      group->targetColors[1] = color2;
      group->targetColors[2] = color1;
      group->state[0]++;
      break;
    
    case 5:
      checkState(group, 2, 0);
      break;

    case 6:
      group->targetColors[2] = color2;
      group->state[0]++;
      break;

    case 7:
      checkState(group, 2, 0);
      break;
      
    default:
      group->waitFrames = wait;
      nextRoad(group);
      group->state[0] = 0;
  }
}

void road(LedGroup* group) {
  roadAnimation(group, 50, currentColor, createColor(0,0,0));
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

int nextUnselected(LedGroup* group) {
  int last = 0;
  
  for(int i=0;i<group->selectionLen; i++) {
    if(group->selected[i] > last) {
      last = group->selected[i];
    }
  }

  return last + 1;
}

int firstUnselected(LedGroup* group) {
  int first = 100;
  
  for(int i=0;i<group->selectionLen; i++) {
    if(group->selected[i] < first) {
      first = group->selected[i];
    }
  }

  return first - 1;
}

void happy(LedGroup* group) {
  byte r = Red(currentColor);
  byte g = Green(currentColor);
  byte b = Blue(currentColor);

  Color highColor1 = createColor(r * 2, g * 2, b * 2);
  Color highColor2 = createColor(r * 5, g * 5, b * 5);

  for(int i=0;i<group->selectionLen; i++) {
  switch(group->state[i]) {
    case 0:
      group->targetColors[i] = highColor1;
      group->state[i]++;
      break;
    
    case 1:
      checkState(group, i, i);
      break;

    case 2:
      if(i == group->selectionLen - 1 && group->selectionLen < 10) {
        group->selected[group->selectionLen] = nextUnselected(group);
        group->state[group->selectionLen] = 0;
        group->selectionLen++;
      }

      group->targetColors[i] = highColor2;
      group->state[i]++;
      
      break;
    
    case 3:
      checkState(group, i, i);
      break;
    
    case 4:
      group->targetColors[i] = currentColor;
      group->state[i]++;
      break;
    
    case 5:
      checkState(group, i, i);
      break;
      
    default:  
      group->state[i] = 0;
      group->selected[i] = nextUnselected(group);

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
 
    group->selected[i] = 0;
    group->state[0] = 0;
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

char* stringAnimation(LedGroup* group) {

  if(group->animation == hide) {
    return "hide";
  }

  if(group->animation == show) {
    return "show";
  }

  if(group->animation == road) {
    return "road";
  }

  if(group->animation == flicker) {
    return "flicker";
  }

  if(group->animation == happy) {
    return "happy";
  }

  return "unknown";
}




