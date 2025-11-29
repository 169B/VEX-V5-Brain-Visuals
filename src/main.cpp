/**
 * @file main.cpp
 * @brief Main Entry Point for VEX V5 Host Mode
 * 
 * This file provides the main entry point for running VEX V5 robot code
 * on a development machine in host mode.
 */

#include "api.h"
#include "host/hal.hpp"
#include "host/ipc.hpp"
#include "host/display.hpp"
#include "auton/selector.hpp"
#include <iostream>
#include <thread>
#include <atomic>
#include <csignal>

// Global state
static std::atomic<bool> running{true};
static std::atomic<host::RobotMode> current_mode{host::RobotMode::DISABLED};

// Signal handler for graceful shutdown
void signal_handler(int signal) {
    std::cout << "\nReceived signal " << signal << ", shutting down..." << std::endl;
    running = false;
}

// User-defined functions (weak implementations - can be overridden)
__attribute__((weak)) void initialize() {
    std::cout << "Default initialize() - override in your code" << std::endl;
    
    // Initialize LCD
    pros::lcd::initialize();
    pros::lcd::print(0, "VEX V5 Host Mode");
    pros::lcd::print(1, "Ready!");
    
    // Initialize autonomous selector
    auton::Selector::instance().init();
}

__attribute__((weak)) void disabled() {
    std::cout << "Robot disabled" << std::endl;
}

__attribute__((weak)) void competition_initialize() {
    std::cout << "Competition initialize" << std::endl;
}

__attribute__((weak)) void autonomous() {
    std::cout << "Autonomous mode started" << std::endl;
    
    // Run selected autonomous
    auton::Selector::instance().run_selected();
    
    std::cout << "Autonomous mode complete" << std::endl;
}

__attribute__((weak)) void opcontrol() {
    std::cout << "Operator control started" << std::endl;
    
    pros::Controller master(pros::E_CONTROLLER_MASTER);
    
    while (current_mode == host::RobotMode::OPCONTROL && running) {
        // Example: Read controller and update LCD
        int left_y = master.get_analog(pros::E_CONTROLLER_ANALOG_LEFT_Y);
        int right_y = master.get_analog(pros::E_CONTROLLER_ANALOG_RIGHT_Y);
        
        pros::lcd::print(3, "L: %4d  R: %4d", left_y, right_y);
        
        // Check for mode button
        if (master.get_digital(pros::E_CONTROLLER_DIGITAL_A)) {
            std::cout << "A button pressed" << std::endl;
        }
        
        pros::delay(20);
    }
    
    std::cout << "Operator control ended" << std::endl;
}

// Mode change handler
void on_mode_change(const std::string& mode) {
    std::cout << "Mode changed to: " << mode << std::endl;
    
    if (mode == "disabled") {
        current_mode = host::RobotMode::DISABLED;
        host::HAL::instance().set_robot_mode(host::RobotMode::DISABLED);
    }
    else if (mode == "autonomous") {
        current_mode = host::RobotMode::AUTONOMOUS;
        host::HAL::instance().set_robot_mode(host::RobotMode::AUTONOMOUS);
    }
    else if (mode == "opcontrol") {
        current_mode = host::RobotMode::OPCONTROL;
        host::HAL::instance().set_robot_mode(host::RobotMode::OPCONTROL);
    }
}

// Touch input handler
void on_touch(const host::TouchInput& input) {
    host::Display::instance().set_touch(input.x, input.y, input.pressed);
}

// Controller input handler
void on_controller(const host::ControllerInput& input) {
    auto& hal = host::HAL::instance();
    hal.set_controller_analog(pros::E_CONTROLLER_MASTER, pros::E_CONTROLLER_ANALOG_LEFT_X, input.lx);
    hal.set_controller_analog(pros::E_CONTROLLER_MASTER, pros::E_CONTROLLER_ANALOG_LEFT_Y, input.ly);
    hal.set_controller_analog(pros::E_CONTROLLER_MASTER, pros::E_CONTROLLER_ANALOG_RIGHT_X, input.rx);
    hal.set_controller_analog(pros::E_CONTROLLER_MASTER, pros::E_CONTROLLER_ANALOG_RIGHT_Y, input.ry);
    
    // Map button bits to digital values
    hal.set_controller_digital(pros::E_CONTROLLER_MASTER, pros::E_CONTROLLER_DIGITAL_A, (input.buttons & 0x01) != 0);
    hal.set_controller_digital(pros::E_CONTROLLER_MASTER, pros::E_CONTROLLER_DIGITAL_B, (input.buttons & 0x02) != 0);
    hal.set_controller_digital(pros::E_CONTROLLER_MASTER, pros::E_CONTROLLER_DIGITAL_X, (input.buttons & 0x04) != 0);
    hal.set_controller_digital(pros::E_CONTROLLER_MASTER, pros::E_CONTROLLER_DIGITAL_Y, (input.buttons & 0x08) != 0);
    hal.set_controller_digital(pros::E_CONTROLLER_MASTER, pros::E_CONTROLLER_DIGITAL_UP, (input.buttons & 0x10) != 0);
    hal.set_controller_digital(pros::E_CONTROLLER_MASTER, pros::E_CONTROLLER_DIGITAL_DOWN, (input.buttons & 0x20) != 0);
    hal.set_controller_digital(pros::E_CONTROLLER_MASTER, pros::E_CONTROLLER_DIGITAL_LEFT, (input.buttons & 0x40) != 0);
    hal.set_controller_digital(pros::E_CONTROLLER_MASTER, pros::E_CONTROLLER_DIGITAL_RIGHT, (input.buttons & 0x80) != 0);
    hal.set_controller_digital(pros::E_CONTROLLER_MASTER, pros::E_CONTROLLER_DIGITAL_L1, (input.buttons & 0x100) != 0);
    hal.set_controller_digital(pros::E_CONTROLLER_MASTER, pros::E_CONTROLLER_DIGITAL_L2, (input.buttons & 0x200) != 0);
    hal.set_controller_digital(pros::E_CONTROLLER_MASTER, pros::E_CONTROLLER_DIGITAL_R1, (input.buttons & 0x400) != 0);
    hal.set_controller_digital(pros::E_CONTROLLER_MASTER, pros::E_CONTROLLER_DIGITAL_R2, (input.buttons & 0x800) != 0);
}

// Auto selection handler
void on_auto_select(const std::string& category, int index) {
    std::cout << "Auto selected: " << category << " #" << index << std::endl;
    
    // This would typically be called from the UI to pre-select an auto
}

// Main function
int main(int argc, char* argv[]) {
    std::cout << "====================================" << std::endl;
    std::cout << "  VEX V5 Host Mode Simulator" << std::endl;
    std::cout << "  PROS Version: " << PROS_VERSION_STRING << std::endl;
    std::cout << "====================================" << std::endl;
    
    // Setup signal handlers
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);
    
    // Parse command line arguments
    std::string server_host = "localhost";
    uint16_t server_port = 9000;
    
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--host" && i + 1 < argc) {
            server_host = argv[++i];
        }
        else if (arg == "--port" && i + 1 < argc) {
            server_port = static_cast<uint16_t>(std::stoi(argv[++i]));
        }
        else if (arg == "--help") {
            std::cout << "Usage: " << argv[0] << " [options]" << std::endl;
            std::cout << "Options:" << std::endl;
            std::cout << "  --host <hostname>  WebSocket server host (default: localhost)" << std::endl;
            std::cout << "  --port <port>      WebSocket server port (default: 9000)" << std::endl;
            std::cout << "  --help             Show this help message" << std::endl;
            return 0;
        }
    }
    
    // Initialize HAL
    std::cout << "Initializing HAL..." << std::endl;
    host::HAL::instance().init();
    
    // Initialize display
    std::cout << "Initializing display..." << std::endl;
    host::Display::instance().init();
    
    // Setup IPC callbacks
    auto& ipc = host::IPCClient::instance();
    ipc.set_touch_callback(on_touch);
    ipc.set_controller_callback(on_controller);
    ipc.set_mode_callback(on_mode_change);
    ipc.set_auto_select_callback(on_auto_select);
    
    // Try to connect to WebSocket server
    std::cout << "Connecting to WebSocket server at " << server_host << ":" << server_port << "..." << std::endl;
    if (!ipc.connect(server_host, server_port)) {
        std::cout << "Warning: Could not connect to WebSocket server." << std::endl;
        std::cout << "Running in standalone mode. Start the UI server with:" << std::endl;
        std::cout << "  cd ui && npm start" << std::endl;
    }
    
    // Run initialization
    std::cout << "\nRunning initialize()..." << std::endl;
    initialize();
    
    // Run competition_initialize
    std::cout << "Running competition_initialize()..." << std::endl;
    competition_initialize();
    
    // Main loop
    std::cout << "\nEntering main loop (Ctrl+C to exit)..." << std::endl;
    std::cout << "Waiting for mode change from UI..." << std::endl;
    
    host::RobotMode last_mode = host::RobotMode::DISABLED;
    std::thread* mode_thread = nullptr;
    
    while (running) {
        // Update HAL (physics simulation)
        host::HAL::instance().update();
        
        // Update display
        host::Display::instance().update();
        
        // Process IPC messages
        ipc.process_messages();
        
        // Check for mode changes
        host::RobotMode mode = current_mode.load();
        if (mode != last_mode) {
            // Stop previous mode thread if running
            if (mode_thread && mode_thread->joinable()) {
                // Wait for previous mode to finish
                // In real PROS, this would be more sophisticated
            }
            
            // Start new mode
            switch (mode) {
                case host::RobotMode::DISABLED:
                    disabled();
                    break;
                    
                case host::RobotMode::AUTONOMOUS:
                    if (mode_thread) delete mode_thread;
                    mode_thread = new std::thread(autonomous);
                    break;
                    
                case host::RobotMode::OPCONTROL:
                    if (mode_thread) delete mode_thread;
                    mode_thread = new std::thread(opcontrol);
                    break;
            }
            
            last_mode = mode;
        }
        
        // Small delay to prevent busy-waiting
        pros::delay(10);
    }
    
    // Cleanup
    std::cout << "\nShutting down..." << std::endl;
    
    current_mode = host::RobotMode::DISABLED;
    
    if (mode_thread) {
        if (mode_thread->joinable()) {
            mode_thread->join();
        }
        delete mode_thread;
    }
    
    ipc.disconnect();
    host::Display::instance().shutdown();
    host::HAL::instance().shutdown();
    
    std::cout << "Goodbye!" << std::endl;
    return 0;
}
