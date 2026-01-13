#ifndef CONFIG_H
#define CONFIG_H

// Relay Expander (PCF8575) Configuration
#define PCF8575_ADDRESS 0x25
// Relays are wired to PCF8575 pins P0-P5 (active LOW)
#define RELAY_STARCH_FEEDER P3
#define RELAY_PAPER_SHREDDER P0
#define RELAY_WATER_PUMP P5
#define RELAY_MIXER P4
#define RELAY_MIXER_DOOR P1
#define RELAY_SPARE P2

// Button Pin Definitions
#define BTN_UP 5
#define BTN_ENTER 7
#define BTN_DOWN 6

// LCD Configuration
#define LCD_ADDRESS 0x27
#define LCD_COLS 20
#define LCD_ROWS 4

// EEPROM Configuration
#define EEPROM_MAGIC_NUMBER 0xAB  // Magic number to verify EEPROM data is valid
#define EEPROM_MAGIC_ADDRESS 0
#define EEPROM_DATA_ADDRESS 1

// Default Timer Values (in milliseconds)
#define DEFAULT_STARCH_TIME 5000    // 5 seconds
#define DEFAULT_PAPER_TIME 10000    // 10 seconds
#define DEFAULT_WATER_TIME 8000     // 8 seconds
#define DEFAULT_MIXING_TIME 30000   // 30 seconds
#define DEFAULT_DOOR_TIME 5000      // 5 seconds

// Timing Constants
#define DEBOUNCE_DELAY 50           // Button debounce delay in ms
#define MIXER_PREP_TIME 2000        // Mixer prep time in ms
#define DOOR_CLOSE_TIME 2000        // Door closing time in ms

// Timer Limits
#define MIN_TIMER_VALUE 1000        // Minimum 1 second
#define MAX_TIMER_VALUE 300000      // Maximum 5 minutes

#endif // CONFIG_H
