# VEX V5 Brain Host Mode Simulator

A complete PROS-style template that enables running VEX V5 robot code on a development machine (host mode) with a Fake Brain UI. This allows testing autonomous routines, auto selectors, and debugging without physical hardware.

## Features

- **LVGL 8.3 Compatible UI**: Uses LVGL types and functions for creating graphical interfaces
- **PROS API Stubs**: Compatible Motor, Controller, Task, and Mutex classes
- **WebSocket IPC**: Real-time communication between host binary and browser UI
- **Autonomous Selector**: Visual tabbed interface for selecting match and skills autonomous routines
- **Motor Telemetry**: Live motor state display in the browser
- **Virtual Controller**: Joysticks and buttons for testing driver control
- **Mode Switching**: Disabled/Autonomous/OpControl mode simulation

## Project Structure

```
├── include/
│   ├── api.h                      # Main PROS-like API header
│   ├── pros/
│   │   ├── motors.hpp             # pros::Motor class
│   │   ├── misc.hpp               # delay, millis
│   │   ├── rtos.hpp               # Task, Mutex
│   │   ├── controller.hpp         # pros::Controller
│   │   └── llemu.hpp              # LCD Emulator
│   ├── liblvgl/
│   │   ├── lvgl.h                 # LVGL types and function stubs
│   │   └── lv_conf.h              # LVGL config (480x272, RGB565)
│   ├── host/
│   │   ├── hal.hpp                # Hardware abstraction layer
│   │   ├── ipc.hpp                # WebSocket IPC client
│   │   └── display.hpp            # LVGL display driver for host
│   └── auton/
│       └── selector.hpp           # Auto selector with LVGL UI
├── src/
│   ├── main.cpp                   # initialize(), autonomous(), opcontrol()
│   ├── pros/                      # PROS API implementations
│   ├── host/                      # Host mode implementations
│   └── auton/                     # Autonomous selector and routines
├── ui/
│   ├── package.json
│   ├── server.js                  # Express + WebSocket server
│   └── public/
│       ├── index.html             # Fake Brain UI
│       ├── styles.css
│       └── app.js                 # Client-side JS
├── Makefile
├── CMakeLists.txt
└── README.md
```

## Quick Start

### Prerequisites

- **C++ Compiler**: GCC 7+ or Clang with C++17 support
- **Node.js**: v14 or later
- **Make** or **CMake**: Build system

### Building

Using Make:
```bash
# Build the host brain executable
make

# Install UI dependencies
make ui-install
```

Using CMake:
```bash
mkdir build && cd build
cmake ..
make
```

### Running

1. **Start the UI server** (in one terminal):
   ```bash
   cd ui
   npm install    # First time only
   npm start
   ```
   
2. **Start the host brain** (in another terminal):
   ```bash
   ./bin/host_brain
   ```
   
3. **Open the UI** in your browser:
   ```
   http://localhost:3000
   ```

### All-in-one (Make):
```bash
make start
```

## Usage

### Writing Robot Code

Your robot code goes in `src/main.cpp`. The familiar PROS functions are available:

```cpp
#include "api.h"

void initialize() {
    pros::lcd::initialize();
    pros::lcd::print(0, "Hello VEX!");
    
    // Initialize autonomous selector
    auton::Selector::instance().init();
}

void autonomous() {
    // Run selected autonomous
    auton::Selector::instance().run_selected();
}

void opcontrol() {
    pros::Controller master(pros::E_CONTROLLER_MASTER);
    pros::Motor left_motor(1);
    pros::Motor right_motor(2);
    
    while (true) {
        int left = master.get_analog(pros::E_CONTROLLER_ANALOG_LEFT_Y);
        int right = master.get_analog(pros::E_CONTROLLER_ANALOG_RIGHT_Y);
        
        left_motor.move(left);
        right_motor.move(right);
        
        pros::delay(20);
    }
}
```

### Registering Autonomous Routines

Use the macros in `src/auton/routines.cpp`:

```cpp
#include "auton/selector.hpp"
#include "api.h"

void my_autonomous() {
    // Your autonomous code here
}

REGISTER_MATCH_AUTO("My Auto", "Description here", my_autonomous);
REGISTER_SKILLS_AUTO("Skills Run", "60 second skills", skills_autonomous);
```

### IPC Protocol

The host binary and UI communicate via WebSocket (port 9000):

**Host → UI:**
```json
{"type":"screen","data":"<base64 RGB565>","x1":0,"y1":0,"x2":479,"y2":271}
{"type":"motor","port":1,"voltage":100,"velocity":200,"position":1500.5}
{"type":"log","level":"info","msg":"Starting autonomous..."}
{"type":"autons","match":[{"name":"Left","desc":"4 rings"}],"skills":[]}
```

**UI → Host:**
```json
{"type":"touch","x":100,"y":50,"pressed":true}
{"type":"controller","analog":{"lx":0,"ly":127},"digital":128}
{"type":"mode","value":"autonomous"}
{"type":"select_auto","category":"match","index":0}
```

## API Reference

### Motor
```cpp
pros::Motor motor(1);  // Port 1
motor.move(127);       // Full power forward
motor.move_velocity(200);  // 200 RPM
double pos = motor.get_position();
double vel = motor.get_actual_velocity();
```

### Controller
```cpp
pros::Controller master(pros::E_CONTROLLER_MASTER);
int left_y = master.get_analog(pros::E_CONTROLLER_ANALOG_LEFT_Y);
bool a_pressed = master.get_digital(pros::E_CONTROLLER_DIGITAL_A);
```

### LCD (LLEMU)
```cpp
pros::lcd::initialize();
pros::lcd::print(0, "Line 1");
pros::lcd::print(1, "Value: %d", 42);
```

### Task
```cpp
pros::Task my_task([]() {
    while (true) {
        // Do something
        pros::delay(10);
    }
});
```

### Mutex
```cpp
pros::Mutex my_mutex;
my_mutex.take();
// Critical section
my_mutex.give();
```

## VEX V5 Brain Specifications

- **Display**: 480×272 pixels, 16-bit color (RGB565)
- **Touch**: Resistive touch screen
- **Ports**: 21 smart ports for motors/sensors
- **Controller**: 2 joysticks, D-pad, 8 face buttons, 4 shoulder buttons

## License

MIT License - See LICENSE file for details.

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.
