# MouldBot v1.0

An automated moulding machine controller built on Arduino Mega 2560 for managing industrial moulding processes with precise timing control and safety features.

## Overview

MouldBot is an embedded system that automates the paper-based moulding process by controlling multiple components through relays and providing an intuitive LCD interface for operation and configuration. The system manages the complete moulding cycle including paper shredding, starch feeding, water pumping, mixing, and door control.

## Features

- **Automated Sequence Control**: Complete automation of the moulding process with customizable timing
- **Interactive LCD Menu**: 20x4 I2C LCD display for real-time status and configuration
- **Persistent Settings**: EEPROM-based storage for timer configurations
- **Test Mode**: Individual component testing for troubleshooting and maintenance
- **Emergency Stop**: Safety feature to halt all operations instantly
- **Debounced Inputs**: Reliable button handling with configurable debounce
- **Modular Design**: Clean separation of concerns with MVC-style architecture

## Hardware Requirements

### Main Components

- **Microcontroller**: Arduino Mega 2560
- **Display**: 20x4 I2C LCD (Address: 0x27)
- **Relay Control**: PCF8575 I2C 16-bit I/O Expander (Address: 0x25)
- **Input**: 3 push buttons (Up, Down, Enter)

### Relays Configuration

The system controls 5 relays through the PCF8575 expander (active LOW):

| Relay | PCF8575 Pin | Function |
|-------|------------|----------|
| Paper Shredder | P0 | Controls paper shredding mechanism |
| Mixer Door | P1 | Opens/closes mixer door |
| Starch Feeder | P3 | Dispenses starch material |
| Mixer | P4 | Controls mixing motor |
| Water Pump | P5 | Controls water dispensing |
| Spare | P2 | Reserved for future use |

### Pin Configuration

```cpp
// Button Pins (Arduino Mega)
UP Button:    Pin 5
ENTER Button: Pin 7
DOWN Button:  Pin 6

// I2C Devices
LCD Address:  0x27
PCF8575 Address: 0x25
```

## Software Dependencies

The project uses PlatformIO with the following libraries:

```ini
platform = atmelavr
board = megaatmega2560
framework = arduino
lib_deps = 
    marcoschwartz/LiquidCrystal_I2C@^1.1.4
    https://github.com/xreef/PCF8575_library.git
```

## Installation

### Prerequisites

- [PlatformIO IDE](https://platformio.org/install/ide?install=vscode) (VS Code extension recommended)
- Arduino Mega 2560 board
- USB cable for programming

### Setup Steps

1. Clone this repository:
   ```bash
   git clone <repository-url>
   cd MouldBot
   ```

2. Open the project in PlatformIO:
   ```bash
   pio run
   ```

3. Upload to Arduino Mega:
   ```bash
   pio run --target upload
   ```

## Configuration

### Timer Settings

All timers can be configured through the LCD menu or by modifying default values in [Config.h](include/Config.h):

| Timer | Default Value | Range | Description |
|-------|--------------|-------|-------------|
| Starch Time | 5 seconds | 1s - 5min | Duration for starch dispensing |
| Paper Time | 10 seconds | 1s - 5min | Duration for paper shredding |
| Water Time | 8 seconds | 1s - 5min | Duration for water pumping |
| Mixing Time | 30 seconds | 1s - 5min | Duration for material mixing |
| Door Time | 5 seconds | 1s - 5min | Duration for door opening |

### Timing Constants

```cpp
DEBOUNCE_DELAY:     50ms   // Button debounce delay
MIXER_PREP_TIME:    2000ms // Mixer preparation time
DOOR_CLOSE_TIME:    2000ms // Door closing duration
```

## Usage

### Main Menu Options

1. **Settings**: Configure timer values for each operation
2. **Run Auto**: Execute the complete automated moulding sequence
3. **Test Machine**: Manually test individual components

### Automated Sequence

The automated process follows this sequence:

1. **Mixer Prep** (2s): Prepares mixer for operation
2. **Paper Shredder**: Shreds paper for configured duration
3. **Starch Feeder**: Dispenses starch for configured duration
4. **Water Pump**: Adds water for configured duration
5. **Mixing**: Mixes materials for configured duration
6. **Moulding Prompt**: Waits for user confirmation to proceed
7. **Door Open**: Opens mixer door for configured duration
8. **Door Close** (2s): Closes mixer door
9. **Complete**: Returns to main menu

### Navigation

- **UP Button**: Navigate up in menus / Increase timer value
- **DOWN Button**: Navigate down in menus / Decrease timer value
- **ENTER Button**: Select menu item / Confirm changes
- **Emergency Stop**: Press all three buttons simultaneously during auto-run to halt immediately

### Test Mode

Test mode allows individual control of each relay:

- Navigate to "Test Machine" from main menu
- Select component to test
- Press ENTER to toggle relay ON/OFF
- Press ENTER on "Back" to return to main menu
- All relays turn OFF when exiting test mode

## Project Structure

```
MouldBot/
├── platformio.ini              # PlatformIO configuration
├── include/
│   ├── Config.h                # Hardware and timing configuration
│   ├── MouldBotController.h    # Main controller class header
│   └── README                  # Include directory info
├── src/
│   ├── main.cpp                # Program entry point
│   └── MouldBotController.cpp  # Controller implementation
├── lib/
│   └── README                  # Library directory info
└── test/
    └── README                  # Test directory info
```

## Architecture

The project follows a clean architecture pattern:

- **main.cpp**: Entry point with setup() and loop() functions
- **MouldBotController**: Main controller class managing state machine
- **Config.h**: Centralized configuration and constants
- **State Management**: Enum-based state machines for menu and auto-run sequences
- **EEPROM Persistence**: Automatic saving/loading of user configurations

## Safety Features

1. **Emergency Stop**: All three buttons pressed together stops all operations
2. **Power Stabilization**: 50ms delays after relay operations
3. **Relay Initialization**: All relays set to OFF state on startup
4. **State Validation**: EEPROM magic number verification
5. **Range Checking**: Timer values constrained to safe limits

## Troubleshooting

### LCD Not Displaying

- Check I2C address (default 0x27)
- Verify I2C connections (SDA/SCL)
- Ensure I2C backpack is properly soldered

### Relays Not Responding

- Verify PCF8575 address (default 0x25)
- Check relay power supply
- Confirm relays are active LOW configuration
- Test in "Test Machine" mode

### Buttons Not Working

- Check pull-up resistors (internal pull-ups enabled)
- Verify pin connections (pins 5, 6, 7)
- Adjust DEBOUNCE_DELAY if needed

### Settings Not Saving

- EEPROM may need initialization (first boot)
- Magic number mismatch - will auto-reset to defaults
- Check EEPROM wear level (limited write cycles)

## Development

### Building

```bash
# Build project
pio run

# Upload to board
pio run --target upload

# Clean build files
pio run --target clean

# Monitor serial output
pio device monitor
```

### Modifying Timers

Edit default values in [Config.h](include/Config.h):

```cpp
#define DEFAULT_STARCH_TIME 5000    // milliseconds
#define DEFAULT_PAPER_TIME 10000
// ... etc
```

### Adding New Relays

1. Define relay pin in [Config.h](include/Config.h)
2. Initialize pin in `MouldBotController::begin()`
3. Add to test menu enum and display
4. Update auto sequence if needed

## License

[Specify your license here]

## Authors

- [Your name/organization]

## Version History

- **v1.0** (2026-01-13): Initial release
  - Automated moulding sequence
  - LCD menu interface
  - EEPROM settings persistence
  - Test mode for components
  - Emergency stop feature

## Acknowledgments

- LiquidCrystal_I2C library by Marco Schwartz
- PCF8575 library by xreef
- PlatformIO ecosystem
