
#include "sound.h"

Buzzer buzzer(11);

void onSound() {
  pinMode(10, OUTPUT);
  analogWrite(10, 3);
  
  buzzer.begin(10);

  buzzer.sound(NOTE_A3, 500); 
}

void initSound() {
  buzzer.sound(NOTE_A3, 500);
  buzzer.sound(NOTE_A3, 500);
  buzzer.sound(NOTE_F3, 375);
  buzzer.sound(NOTE_C4, 125);

  buzzer.sound(NOTE_A3, 500);
  buzzer.sound(NOTE_F3, 375);
  buzzer.sound(NOTE_C4, 125);
  buzzer.sound(NOTE_A3, 1000);
}

void connectedSound() {
  buzzer.sound(NOTE_E4, 500); 
  buzzer.sound(NOTE_E4, 500);
  buzzer.sound(NOTE_E4, 500);
  buzzer.sound(NOTE_F4, 375);
  buzzer.sound(NOTE_C4, 125);

  buzzer.sound(NOTE_GS3, 500);
  buzzer.sound(NOTE_F3, 375);
  buzzer.sound(NOTE_C4, 125);
  buzzer.sound(NOTE_A3, 1000);
}

void readySound() {
  buzzer.sound(NOTE_A4, 500);
  buzzer.sound(NOTE_A3, 375);
  buzzer.sound(NOTE_A3, 125);
  buzzer.sound(NOTE_A4, 500);
  buzzer.sound(NOTE_GS4, 375);
  buzzer.sound(NOTE_G4, 125);
}