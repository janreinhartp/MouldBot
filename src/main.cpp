#include <Arduino.h>
#include "MouldBotController.h"

// Global controller instance
MouldBotController controller;

void setup() {
  controller.begin();
}

void loop() {
  controller.update();
}