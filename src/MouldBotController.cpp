#include "MouldBotController.h"
#include <EEPROM.h>

MouldBotController::MouldBotController() : lcd(LCD_ADDRESS, LCD_COLS, LCD_ROWS), relayExpander(PCF8575_ADDRESS)
{
    currentState = MAIN_MENU;
    currentMenuIndex = 0;
    editingTimer = -1;

    lastUpState = HIGH;
    lastEnterState = HIGH;
    lastDownState = HIGH;
  lastUpDebounceTime = 0;
  lastEnterDebounceTime = 0;
  lastDownDebounceTime = 0;

    for (int i = 0; i < 5; i++)
    {
        relayStates[i] = false;
    }
}

void MouldBotController::begin()
{
    // Ensure I2C is up before talking to LCD or PCF8575
    Wire.begin();

    // Initialize LCD
    lcd.init();
    lcd.backlight();

    // Initialize relay expander pins (active LOW relays)
    relayExpander.begin();
    relayExpander.pinMode(RELAY_STARCH_FEEDER, OUTPUT);
    relayExpander.pinMode(RELAY_PAPER_SHREDDER, OUTPUT);
    relayExpander.pinMode(RELAY_WATER_PUMP, OUTPUT);
    relayExpander.pinMode(RELAY_MIXER, OUTPUT);
    relayExpander.pinMode(RELAY_MIXER_DOOR, OUTPUT);
    relayExpander.pinMode(RELAY_SPARE, OUTPUT);

    pinMode(BTN_UP, INPUT_PULLUP);
    pinMode(BTN_ENTER, INPUT_PULLUP);
    pinMode(BTN_DOWN, INPUT_PULLUP);

    // Turn off all relays
    allRelaysOff();

    // Show welcome message
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("   MouldBot v1.0   ");
    lcd.setCursor(0, 1);
    lcd.print("  Initializing...  ");
    delay(1000);

    // Load timers from EEPROM
    lcd.setCursor(0, 2);
    lcd.print("Loading Settings... ");
    loadTimersFromEEPROM();
    delay(1000);

    displayMainMenu();
}

void MouldBotController::update()
{
    handleButtons();

    if (currentState == RUN_AUTO && autoRunning)
    {
        handleAutoSequence();
    }
}

void MouldBotController::allRelaysOff()
{
    relayExpander.digitalWrite(RELAY_STARCH_FEEDER, HIGH);
    delay(50);
    relayExpander.digitalWrite(RELAY_PAPER_SHREDDER, HIGH);
    delay(50);
    relayExpander.digitalWrite(RELAY_WATER_PUMP, HIGH);
    delay(50);
    relayExpander.digitalWrite(RELAY_MIXER, HIGH);
    delay(50);
    relayExpander.digitalWrite(RELAY_MIXER_DOOR, HIGH);
    delay(50);
    relayExpander.digitalWrite(RELAY_SPARE, HIGH);
    delay(50);
}

void MouldBotController::setRelay(int pin, bool state)
{
    relayExpander.digitalWrite(pin, state ? LOW : HIGH);
    delay(50);  // Delay after relay operation to stabilize power
}

void MouldBotController::handleButtons()
{
    unsigned long currentTime = millis();
    
    // Read current button states
    bool upReading = digitalRead(BTN_UP) == LOW;
    bool enterReading = digitalRead(BTN_ENTER) == LOW;
    bool downReading = digitalRead(BTN_DOWN) == LOW;

    // Emergency stop - all 3 buttons pressed during auto run
    if (currentState == RUN_AUTO && autoRunning && upReading && enterReading && downReading)
    {
        autoRunning = false;
        allRelaysOff();
        currentState = MAIN_MENU;
        currentMenuIndex = 0;
        lcd.clear();
        lcd.setCursor(0, 1);
        lcd.print("AUTO RUN STOPPED!");
        delay(1500);
        displayMainMenu();
        return;
    }

    // Handle UP button - detect press on falling edge with debounce
    if (upReading && !lastUpState)
    {
        if ((currentTime - lastUpDebounceTime) > DEBOUNCE_DELAY)
        {
            onUpPressed();
            lastUpDebounceTime = currentTime;
        }
    }
    lastUpState = upReading;

    // Handle ENTER button - detect press on falling edge with debounce
    if (enterReading && !lastEnterState)
    {
        if ((currentTime - lastEnterDebounceTime) > DEBOUNCE_DELAY)
        {
            onEnterPressed();
            lastEnterDebounceTime = currentTime;
        }
    }
    lastEnterState = enterReading;

    // Handle DOWN button - detect press on falling edge with debounce
    if (downReading && !lastDownState)
    {
        if ((currentTime - lastDownDebounceTime) > DEBOUNCE_DELAY)
        {
            onDownPressed();
            lastDownDebounceTime = currentTime;
        }
    }
    lastDownState = downReading;
}

void MouldBotController::onUpPressed()
{
    if (currentState == MAIN_MENU)
    {
        currentMenuIndex = (currentMenuIndex - 1 + MAIN_MENU_COUNT) % MAIN_MENU_COUNT;
        displayMainMenu();
    }
    else if (currentState == SETTINGS_MENU)
    {
        currentMenuIndex = (currentMenuIndex - 1 + SETTINGS_COUNT) % SETTINGS_COUNT;
        displaySettingsMenu();
    }
    else if (currentState == TEST_MACHINE)
    {
        currentMenuIndex = (currentMenuIndex - 1 + TEST_COUNT) % TEST_COUNT;
        displayTestMenu();
    }
    else if (currentState == EDIT_TIMER)
    {
        timerEditValue += 1000; // Increase by 1 second
        if (timerEditValue > MAX_TIMER_VALUE)
            timerEditValue = MAX_TIMER_VALUE;
        displayTimerEdit();
    }
}

void MouldBotController::onDownPressed()
{
    if (currentState == MAIN_MENU)
    {
        currentMenuIndex = (currentMenuIndex + 1) % MAIN_MENU_COUNT;
        displayMainMenu();
    }
    else if (currentState == SETTINGS_MENU)
    {
        currentMenuIndex = (currentMenuIndex + 1) % SETTINGS_COUNT;
        displaySettingsMenu();
    }
    else if (currentState == TEST_MACHINE)
    {
        currentMenuIndex = (currentMenuIndex + 1) % TEST_COUNT;
        displayTestMenu();
    }
    else if (currentState == EDIT_TIMER)
    {
        timerEditValue -= 1000; // Decrease by 1 second
        if (timerEditValue < MIN_TIMER_VALUE)
            timerEditValue = MIN_TIMER_VALUE;
        displayTimerEdit();
    }
}

void MouldBotController::onEnterPressed()
{
    if (currentState == MAIN_MENU)
    {
        switch (currentMenuIndex)
        {
        case SETTINGS:
            currentState = SETTINGS_MENU;
            currentMenuIndex = 0;
            displaySettingsMenu();
            break;
        case RUN_AUTO_OPTION:
            startAutoRun();
            break;
        case TEST_MACHINE_OPTION:
            currentState = TEST_MACHINE;
            currentMenuIndex = 0;
            displayTestMenu();
            break;
        }
    }
    else if (currentState == SETTINGS_MENU)
    {
        if (currentMenuIndex == BACK_TO_MAIN)
        {
            currentState = MAIN_MENU;
            currentMenuIndex = 0;
            displayMainMenu();
        }
        else
        {
            enterTimerEdit(currentMenuIndex);
        }
    }
    else if (currentState == TEST_MACHINE)
    {
        if (currentMenuIndex == BACK_FROM_TEST)
        {
            allRelaysOff();
            for (int i = 0; i < 5; i++)
                relayStates[i] = false;
            currentState = MAIN_MENU;
            currentMenuIndex = 0;
            displayMainMenu();
        }
        else
        {
            toggleTestRelay(currentMenuIndex);
        }
    }
    else if (currentState == EDIT_TIMER)
    {
        saveTimerEdit();
        saveTimersToEEPROM(); // Save to EEPROM when timer is changed
        currentState = SETTINGS_MENU;
        displaySettingsMenu();
    }
    else if (currentState == RUN_AUTO)
    {
        if (autoState == AUTO_MOULDING_PROMPT)
        {
            // User pressed enter to continue moulding
            autoState = AUTO_DOOR_OPEN;
            stateStartTime = millis();
            setRelay(RELAY_MIXER_DOOR, true); // Open door
            displayAutoStatus();
        }
        else if (autoState == AUTO_COMPLETE)
        {
            // Return to main menu
            autoRunning = false;
            allRelaysOff();
            currentState = MAIN_MENU;
            currentMenuIndex = 0;
            displayMainMenu();
        }
    }
}

void MouldBotController::displayMainMenu()
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("==== MAIN MENU ====");

    lcd.setCursor(0, 1);
    lcd.print(currentMenuIndex == SETTINGS ? "> " : "  ");
    lcd.print("Settings");

    lcd.setCursor(0, 2);
    lcd.print(currentMenuIndex == RUN_AUTO_OPTION ? "> " : "  ");
    lcd.print("Run Auto");

    lcd.setCursor(0, 3);
    lcd.print(currentMenuIndex == TEST_MACHINE_OPTION ? "> " : "  ");
    lcd.print("Test Machine");
}

void MouldBotController::displaySettingsMenu()
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("===== SETTINGS =====");

    int startIdx = currentMenuIndex;
    if (startIdx > 3)
        startIdx = 3;

    for (int i = 0; i < 3 && (startIdx + i - currentMenuIndex + currentMenuIndex) < SETTINGS_COUNT; i++)
    {
        int idx = startIdx + i;
        if (idx >= SETTINGS_COUNT)
            break;

        lcd.setCursor(0, i + 1);
        lcd.print(idx == currentMenuIndex ? "> " : "  ");

        switch (idx)
        {
        case STARCH_TIMER:
            lcd.print("Starch:");
            lcd.print(timers.starchOnTime / 1000);
            lcd.print("s");
            break;
        case PAPER_TIMER:
            lcd.print("Paper:");
            lcd.print(timers.paperOnTime / 1000);
            lcd.print("s");
            break;
        case WATER_TIMER:
            lcd.print("Water:");
            lcd.print(timers.waterPumpTime / 1000);
            lcd.print("s");
            break;
        case MIXING_TIMER:
            lcd.print("Mixing:");
            lcd.print(timers.mixingTime / 1000);
            lcd.print("s");
            break;
        case DOOR_TIMER:
            lcd.print("Door:");
            lcd.print(timers.doorOpenTime / 1000);
            lcd.print("s");
            break;
        case BACK_TO_MAIN:
            lcd.print("Back");
            break;
        }
    }
}

void MouldBotController::displayTestMenu()
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("==== TEST MODE ====");

    int startIdx = currentMenuIndex;
    if (startIdx > 3)
        startIdx = 3;

    for (int i = 0; i < 3 && (startIdx + i) < TEST_COUNT; i++)
    {
        int idx = startIdx + i;
        if (idx >= TEST_COUNT)
            break;

        lcd.setCursor(0, i + 1);
        lcd.print(idx == currentMenuIndex ? "> " : "  ");

        switch (idx)
        {
        case TEST_STARCH:
            lcd.print("Starch:");
            lcd.print(relayStates[0] ? "ON " : "OFF");
            break;
        case TEST_PAPER:
            lcd.print("Paper:");
            lcd.print(relayStates[1] ? "ON " : "OFF");
            break;
        case TEST_WATER:
            lcd.print("Water:");
            lcd.print(relayStates[2] ? "ON " : "OFF");
            break;
        case TEST_MIXER:
            lcd.print("Mixer:");
            lcd.print(relayStates[3] ? "ON " : "OFF");
            break;
        case TEST_DOOR:
            lcd.print("Door:");
            lcd.print(relayStates[4] ? "OPEN" : "CLOSE");
            break;
        case BACK_FROM_TEST:
            lcd.print("Back");
            break;
        }
    }
}

void MouldBotController::enterTimerEdit(int timerIndex)
{
    editingTimer = timerIndex;
    currentState = EDIT_TIMER;

    switch (timerIndex)
    {
    case STARCH_TIMER:
        timerEditValue = timers.starchOnTime;
        break;
    case PAPER_TIMER:
        timerEditValue = timers.paperOnTime;
        break;
    case WATER_TIMER:
        timerEditValue = timers.waterPumpTime;
        break;
    case MIXING_TIMER:
        timerEditValue = timers.mixingTime;
        break;
    case DOOR_TIMER:
        timerEditValue = timers.doorOpenTime;
        break;
    }

    displayTimerEdit();
}

void MouldBotController::displayTimerEdit()
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("=== EDIT TIMER ===");

    lcd.setCursor(0, 1);
    switch (editingTimer)
    {
    case STARCH_TIMER:
        lcd.print("Starch Timer");
        break;
    case PAPER_TIMER:
        lcd.print("Paper Timer");
        break;
    case WATER_TIMER:
        lcd.print("Water Timer");
        break;
    case MIXING_TIMER:
        lcd.print("Mixing Timer");
        break;
    case DOOR_TIMER:
        lcd.print("Door Timer");
        break;
    }

    lcd.setCursor(0, 2);
    lcd.print("Value: ");
    lcd.print(timerEditValue / 1000);
    lcd.print(" sec");

    lcd.setCursor(0, 3);
    lcd.print("Up/Down: +/-1s");
}

void MouldBotController::saveTimerEdit()
{
    switch (editingTimer)
    {
    case STARCH_TIMER:
        timers.starchOnTime = timerEditValue;
        break;
    case PAPER_TIMER:
        timers.paperOnTime = timerEditValue;
        break;
    case WATER_TIMER:
        timers.waterPumpTime = timerEditValue;
        break;
    case MIXING_TIMER:
        timers.mixingTime = timerEditValue;
        break;
    case DOOR_TIMER:
        timers.doorOpenTime = timerEditValue;
        break;
    }
    editingTimer = -1;
}

void MouldBotController::toggleTestRelay(int relayIndex)
{
    relayStates[relayIndex] = !relayStates[relayIndex];

    switch (relayIndex)
    {
    case TEST_STARCH:
        setRelay(RELAY_STARCH_FEEDER, relayStates[relayIndex]);
        break;
    case TEST_PAPER:
        setRelay(RELAY_PAPER_SHREDDER, relayStates[relayIndex]);
        break;
    case TEST_WATER:
        setRelay(RELAY_WATER_PUMP, relayStates[relayIndex]);
        break;
    case TEST_MIXER:
        setRelay(RELAY_MIXER, relayStates[relayIndex]);
        break;
    case TEST_DOOR:
        setRelay(RELAY_MIXER_DOOR, relayStates[relayIndex]);
        break;
    }

    displayTestMenu();
}

void MouldBotController::startAutoRun()
{
    currentState = RUN_AUTO;
    autoRunning = true;
    autoState = AUTO_MIXER_PREP;
    stateStartTime = millis();
    allRelaysOff();

    setRelay(RELAY_MIXER, true); // Turn on mixer for prep
    displayAutoStatus();
}

void MouldBotController::handleAutoSequence()
{
    unsigned long currentTime = millis();
    unsigned long elapsed = currentTime - stateStartTime;

    switch (autoState)
    {
    case AUTO_IDLE:
        // Idle state; nothing to do until auto run starts
        break;
    case AUTO_MIXER_PREP:
        if (elapsed >= MIXER_PREP_TIME)
        {
            autoState = AUTO_PAPER_SHREDDER;
            stateStartTime = currentTime;
            setRelay(RELAY_PAPER_SHREDDER, true);
            displayAutoStatus();
        }
        break;

    case AUTO_PAPER_SHREDDER:
        if (elapsed >= timers.paperOnTime)
        {
            setRelay(RELAY_PAPER_SHREDDER, false);
            delay(100);  // Delay between relay switches
            autoState = AUTO_STARCH_FEEDER;
            stateStartTime = currentTime;
            setRelay(RELAY_STARCH_FEEDER, true);
            displayAutoStatus();
        }
        break;

    case AUTO_STARCH_FEEDER:
        if (elapsed >= timers.starchOnTime)
        {
            setRelay(RELAY_STARCH_FEEDER, false);
            delay(100);  // Delay between relay switches
            autoState = AUTO_WATER_PUMP;
            stateStartTime = currentTime;
            setRelay(RELAY_WATER_PUMP, true);
            displayAutoStatus();
        }
        break;

    case AUTO_WATER_PUMP:
        if (elapsed >= timers.waterPumpTime)
        {
            setRelay(RELAY_WATER_PUMP, false);
            autoState = AUTO_MIXING;
            stateStartTime = currentTime;
            displayAutoStatus();
        }
        break;

    case AUTO_MIXING:
        if (elapsed >= timers.mixingTime)
        {
            // Keep mixer running, don't turn it off
            autoState = AUTO_MOULDING_PROMPT;
            displayAutoStatus();
        }
        break;

    case AUTO_MOULDING_PROMPT:
        // Waiting for user to press enter
        break;

    case AUTO_DOOR_OPEN:
        if (elapsed >= timers.doorOpenTime)
        {
            setRelay(RELAY_MIXER_DOOR, false);
            autoState = AUTO_DOOR_CLOSE;
            stateStartTime = currentTime;
            displayAutoStatus();
        }
        break;

    case AUTO_DOOR_CLOSE:
        if (elapsed >= DOOR_CLOSE_TIME)
        {
            autoState = AUTO_MOULDING_PROMPT; // Repeat moulding
            displayAutoStatus();
        }
        break;
    case AUTO_COMPLETE:
        // Completed sequence; waiting for user acknowledgement
        break;
    }

    // Update display every second during timed operations
    if (autoState != AUTO_MOULDING_PROMPT && autoState != AUTO_COMPLETE)
    {
        static unsigned long lastUpdate = 0;
        if (currentTime - lastUpdate >= 1000)
        {
            displayAutoStatus();
            lastUpdate = currentTime;
        }
    }
}

void MouldBotController::displayAutoStatus()
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("=== AUTO RUNNING ===");

    unsigned long elapsed = millis() - stateStartTime;
    unsigned long remaining = 0;

    lcd.setCursor(0, 1);
    switch (autoState)
    {
    case AUTO_IDLE:
        lcd.print("Status: Idle");
        remaining = 0;
        break;
    case AUTO_MIXER_PREP:
        lcd.print("Status: Mixer Prep");
        remaining = (MIXER_PREP_TIME - elapsed) / 1000;
        break;
    case AUTO_PAPER_SHREDDER:
        lcd.print("Status: Paper Feed");
        remaining = (timers.paperOnTime - elapsed) / 1000;
        break;
    case AUTO_STARCH_FEEDER:
        lcd.print("Status: Starch Feed");
        remaining = (timers.starchOnTime - elapsed) / 1000;
        break;
    case AUTO_WATER_PUMP:
        lcd.print("Status: Water Pump");
        remaining = (timers.waterPumpTime - elapsed) / 1000;
        break;
    case AUTO_MIXING:
        lcd.print("Status: Mixing");
        remaining = (timers.mixingTime - elapsed) / 1000;
        break;
    case AUTO_MOULDING_PROMPT:
        lcd.print("Add Mould & Press");
        lcd.setCursor(0, 2);
        lcd.print("ENTER to continue");
        return;
    case AUTO_DOOR_OPEN:
        lcd.print("Status: Door Open");
        remaining = (timers.doorOpenTime - elapsed) / 1000;
        break;
    case AUTO_DOOR_CLOSE:
        lcd.print("Status: Door Close");
        remaining = (DOOR_CLOSE_TIME - elapsed) / 1000;
        break;
    case AUTO_COMPLETE:
        lcd.print("Status: Complete");
        lcd.setCursor(0, 2);
        lcd.print("Press ENTER");
        return;
    }

    if (autoState != AUTO_MOULDING_PROMPT)
    {
        lcd.setCursor(0, 2);
        lcd.print("Time Left: ");
        lcd.print(remaining);
        lcd.print("s  ");
    }
}

void MouldBotController::loadTimersFromEEPROM()
{
    // Check if EEPROM has valid data
    byte magicNumber = EEPROM.read(EEPROM_MAGIC_ADDRESS);

    if (magicNumber == EEPROM_MAGIC_NUMBER)
    {
        // Valid data exists, load it
        int address = EEPROM_DATA_ADDRESS;

        EEPROM.get(address, timers.starchOnTime);
        address += sizeof(unsigned long);

        EEPROM.get(address, timers.paperOnTime);
        address += sizeof(unsigned long);

        EEPROM.get(address, timers.waterPumpTime);
        address += sizeof(unsigned long);

        EEPROM.get(address, timers.mixingTime);
        address += sizeof(unsigned long);

        EEPROM.get(address, timers.doorOpenTime);
    }
    else
    {
        // No valid data, use defaults
        setDefaultTimers();
        saveTimersToEEPROM(); // Save defaults to EEPROM
    }
}

void MouldBotController::saveTimersToEEPROM()
{
    // Write magic number to indicate valid data
    EEPROM.write(EEPROM_MAGIC_ADDRESS, EEPROM_MAGIC_NUMBER);

    // Write timer values
    int address = EEPROM_DATA_ADDRESS;

    EEPROM.put(address, timers.starchOnTime);
    address += sizeof(unsigned long);

    EEPROM.put(address, timers.paperOnTime);
    address += sizeof(unsigned long);

    EEPROM.put(address, timers.waterPumpTime);
    address += sizeof(unsigned long);

    EEPROM.put(address, timers.mixingTime);
    address += sizeof(unsigned long);

    EEPROM.put(address, timers.doorOpenTime);
}

void MouldBotController::setDefaultTimers()
{
    timers.starchOnTime = DEFAULT_STARCH_TIME;
    timers.paperOnTime = DEFAULT_PAPER_TIME;
    timers.waterPumpTime = DEFAULT_WATER_TIME;
    timers.mixingTime = DEFAULT_MIXING_TIME;
    timers.doorOpenTime = DEFAULT_DOOR_TIME;
}
