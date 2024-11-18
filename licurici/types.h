#ifndef LICURICI_TYPES
#define LICURICI_TYPES

#define TOTAL_GROUPS 2

typedef size_t (*StringOut)(const char[]);
typedef size_t (*IntOut)(int);
typedef int (*IntIn)();

typedef void (*FlushReport)(StringOut, IntOut);

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

typedef void (*PerformAction)(SerialAction, StringOut, IntOut, IntIn);

SerialAction intToAction(int value);
#endif