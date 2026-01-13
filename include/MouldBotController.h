#ifndef MOULDBOTCONTROLLER_H
#define MOULDBOTCONTROLLER_H

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <PCF8575.h>
#include "Config.h"

// Timer Structure
struct Timers {
  unsigned long starchOnTime;
  unsigned long paperOnTime;
  unsigned long waterPumpTime;
  unsigned long mixingTime;
  unsigned long doorOpenTime;
};

class MouldBotController {
private:
  LiquidCrystal_I2C lcd;
  PCF8575 relayExpander;
  Timers timers;
  
  // Menu state enums
  enum MenuState {
    MAIN_MENU,
    SETTINGS_MENU,
    RUN_AUTO,
    TEST_MACHINE,
    EDIT_TIMER
  };
  
  enum MainMenuOption {
    SETTINGS,
    RUN_AUTO_OPTION,
    TEST_MACHINE_OPTION,
    MAIN_MENU_COUNT
  };
  
  enum SettingsOption {
    STARCH_TIMER,
    PAPER_TIMER,
    WATER_TIMER,
    MIXING_TIMER,
    DOOR_TIMER,
    BACK_TO_MAIN,
    SETTINGS_COUNT
  };
  
  enum TestOption {
    TEST_STARCH,
    TEST_PAPER,
    TEST_WATER,
    TEST_MIXER,
    TEST_DOOR,
    BACK_FROM_TEST,
    TEST_COUNT
  };
  
  enum AutoState {
    AUTO_IDLE,
    AUTO_MIXER_PREP,
    AUTO_PAPER_SHREDDER,
    AUTO_STARCH_FEEDER,
    AUTO_WATER_PUMP,
    AUTO_MIXING,
    AUTO_MOULDING_PROMPT,
    AUTO_DOOR_OPEN,
    AUTO_DOOR_CLOSE,
    AUTO_COMPLETE
  };
  
  // State variables
  MenuState currentState;
  int currentMenuIndex;
  int editingTimer;
  unsigned long timerEditValue;
  
  // Button state
  bool lastUpState;
  bool lastEnterState;
  bool lastDownState;
  unsigned long lastUpDebounceTime;
  unsigned long lastEnterDebounceTime;
  unsigned long lastDownDebounceTime;
  
  // Auto run state
  AutoState autoState;
  unsigned long stateStartTime;
  bool autoRunning;
  
  // Relay states for test mode
  bool relayStates[5];
  
  // Private methods
  void allRelaysOff();
  void setRelay(int pin, bool state);
  void handleButtons();
  void onUpPressed();
  void onDownPressed();
  void onEnterPressed();
  
  // Display methods
  void displayMainMenu();
  void displaySettingsMenu();
  void displayTestMenu();
  void displayTimerEdit();
  void displayAutoStatus();
  
  // Settings methods
  void enterTimerEdit(int timerIndex);
  void saveTimerEdit();
  
  // Test mode methods
  void toggleTestRelay(int relayIndex);
  
  // Auto run methods
  void startAutoRun();
  void handleAutoSequence();
  
  // EEPROM methods
  void loadTimersFromEEPROM();
  void saveTimersToEEPROM();
  void setDefaultTimers();

public:
  MouldBotController();
  void begin();
  void update();
};

#endif // MOULDBOTCONTROLLER_H
