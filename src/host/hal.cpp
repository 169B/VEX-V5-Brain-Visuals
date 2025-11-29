/**
 * @file hal.cpp
 * @brief Hardware Abstraction Layer Implementation for Host Mode
 */

#include "host/hal.hpp"
#include <algorithm>
#include <cmath>

namespace host {

// Singleton instance
HAL& HAL::instance() {
    static HAL instance;
    return instance;
}

HAL::HAL() : _robot_mode(RobotMode::DISABLED), _competition_connected(false) {
    init();
}

HAL::~HAL() {
    shutdown();
}

void HAL::init() {
    std::lock_guard<std::mutex> lock(_mutex);
    
    // Initialize motors
    for (auto& motor : _motors) {
        motor = MotorState();
    }
    
    // Initialize controllers
    for (auto& controller : _controllers) {
        controller = ControllerState();
        controller.connected = true; // Default to connected
    }
    
    // Initialize battery
    _battery = BatteryState();
    
    // Initialize LCD
    for (auto& line : _lcd_lines) {
        line.clear();
    }
    _lcd_buttons = 0;
    _lcd_initialized = false;
}

void HAL::shutdown() {
    std::lock_guard<std::mutex> lock(_mutex);
    _robot_mode = RobotMode::DISABLED;
}

void HAL::update() {
    std::lock_guard<std::mutex> lock(_mutex);
    
    // Simulate motor physics
    for (auto& motor : _motors) {
        if (!motor.connected) continue;
        
        // Calculate max velocity based on gearset
        double max_velocity = 200.0; // Default 18:1
        switch (motor.gearset) {
            case pros::E_MOTOR_GEARSET_36: max_velocity = 100.0; break;
            case pros::E_MOTOR_GEARSET_18: max_velocity = 200.0; break;
            case pros::E_MOTOR_GEARSET_06: max_velocity = 600.0; break;
            default: break;
        }
        
        // Calculate target velocity from voltage
        double target_velocity = (motor.voltage / 127.0) * max_velocity;
        
        // Smooth velocity change (simple first-order filter)
        double alpha = 0.1;
        motor.actual_velocity = motor.actual_velocity * (1.0 - alpha) + target_velocity * alpha;
        
        // Update position based on velocity (assuming 10ms update rate)
        motor.position += motor.actual_velocity * (10.0 / 60000.0) * 360.0; // degrees
        
        // Simulate current draw
        motor.current = static_cast<int32_t>(std::abs(motor.actual_velocity / max_velocity) * 2000);
        
        // Simulate temperature
        motor.temperature = 25.0 + (std::abs(motor.current) / 2500.0) * 30.0;
    }
    
    // Notify callback if registered
    if (_state_callback) {
        _state_callback();
    }
}

// Motor functions
void HAL::set_motor(uint8_t port, int32_t voltage) {
    if (port < 1 || port > 21) return;
    std::lock_guard<std::mutex> lock(_mutex);
    _motors[port - 1].voltage = std::clamp(voltage, -127, 127);
}

void HAL::set_motor_velocity(uint8_t port, int32_t velocity) {
    if (port < 1 || port > 21) return;
    std::lock_guard<std::mutex> lock(_mutex);
    _motors[port - 1].velocity = velocity;
}

void HAL::set_motor_position(uint8_t port, double position) {
    if (port < 1 || port > 21) return;
    std::lock_guard<std::mutex> lock(_mutex);
    _motors[port - 1].position = position;
}

void HAL::set_motor_gearset(uint8_t port, pros::motor_gearset_e_t gearset) {
    if (port < 1 || port > 21) return;
    std::lock_guard<std::mutex> lock(_mutex);
    _motors[port - 1].gearset = gearset;
}

void HAL::set_motor_reversed(uint8_t port, bool reversed) {
    if (port < 1 || port > 21) return;
    std::lock_guard<std::mutex> lock(_mutex);
    _motors[port - 1].reversed = reversed;
}

void HAL::set_motor_connected(uint8_t port, bool connected) {
    if (port < 1 || port > 21) return;
    std::lock_guard<std::mutex> lock(_mutex);
    _motors[port - 1].connected = connected;
}

int32_t HAL::get_motor_voltage(uint8_t port) {
    if (port < 1 || port > 21) return 0;
    std::lock_guard<std::mutex> lock(_mutex);
    return _motors[port - 1].voltage;
}

int32_t HAL::get_motor_velocity(uint8_t port) {
    if (port < 1 || port > 21) return 0;
    std::lock_guard<std::mutex> lock(_mutex);
    return _motors[port - 1].velocity;
}

double HAL::get_motor_position(uint8_t port) {
    if (port < 1 || port > 21) return 0.0;
    std::lock_guard<std::mutex> lock(_mutex);
    return _motors[port - 1].position;
}

double HAL::get_motor_actual_velocity(uint8_t port) {
    if (port < 1 || port > 21) return 0.0;
    std::lock_guard<std::mutex> lock(_mutex);
    return _motors[port - 1].actual_velocity;
}

int32_t HAL::get_motor_current(uint8_t port) {
    if (port < 1 || port > 21) return 0;
    std::lock_guard<std::mutex> lock(_mutex);
    return _motors[port - 1].current;
}

double HAL::get_motor_temperature(uint8_t port) {
    if (port < 1 || port > 21) return 0.0;
    std::lock_guard<std::mutex> lock(_mutex);
    return _motors[port - 1].temperature;
}

pros::motor_gearset_e_t HAL::get_motor_gearset(uint8_t port) {
    if (port < 1 || port > 21) return pros::E_MOTOR_GEARSET_INVALID;
    std::lock_guard<std::mutex> lock(_mutex);
    return _motors[port - 1].gearset;
}

bool HAL::get_motor_reversed(uint8_t port) {
    if (port < 1 || port > 21) return false;
    std::lock_guard<std::mutex> lock(_mutex);
    return _motors[port - 1].reversed;
}

bool HAL::is_motor_connected(uint8_t port) {
    if (port < 1 || port > 21) return false;
    std::lock_guard<std::mutex> lock(_mutex);
    return _motors[port - 1].connected;
}

// Controller functions
void HAL::set_controller_analog(pros::controller_id_e_t id, pros::controller_analog_e_t channel, int32_t value) {
    if (id > 1 || channel > 3) return;
    std::lock_guard<std::mutex> lock(_mutex);
    _controllers[id].analog[channel] = std::clamp(value, -127, 127);
}

void HAL::set_controller_digital(pros::controller_id_e_t id, pros::controller_digital_e_t button, bool value) {
    if (id > 1 || button >= 18) return;
    std::lock_guard<std::mutex> lock(_mutex);
    _controllers[id].digital[button] = value;
}

void HAL::set_controller_connected(pros::controller_id_e_t id, bool connected) {
    if (id > 1) return;
    std::lock_guard<std::mutex> lock(_mutex);
    _controllers[id].connected = connected;
}

int32_t HAL::get_controller_analog(pros::controller_id_e_t id, pros::controller_analog_e_t channel) {
    if (id > 1 || channel > 3) return 0;
    std::lock_guard<std::mutex> lock(_mutex);
    return _controllers[id].analog[channel];
}

bool HAL::get_controller_digital(pros::controller_id_e_t id, pros::controller_digital_e_t button) {
    if (id > 1 || button >= 18) return false;
    std::lock_guard<std::mutex> lock(_mutex);
    return _controllers[id].digital[button];
}

bool HAL::is_controller_connected(pros::controller_id_e_t id) {
    if (id > 1) return false;
    std::lock_guard<std::mutex> lock(_mutex);
    return _controllers[id].connected;
}

int32_t HAL::get_controller_battery_capacity(pros::controller_id_e_t id) {
    if (id > 1) return 0;
    std::lock_guard<std::mutex> lock(_mutex);
    return _controllers[id].battery_capacity;
}

int32_t HAL::get_controller_battery_level(pros::controller_id_e_t id) {
    if (id > 1) return 0;
    std::lock_guard<std::mutex> lock(_mutex);
    return _controllers[id].battery_level;
}

// Battery functions
double HAL::get_battery_capacity() {
    std::lock_guard<std::mutex> lock(_mutex);
    return _battery.capacity;
}

int32_t HAL::get_battery_current() {
    std::lock_guard<std::mutex> lock(_mutex);
    return _battery.current;
}

double HAL::get_battery_temperature() {
    std::lock_guard<std::mutex> lock(_mutex);
    return _battery.temperature;
}

int32_t HAL::get_battery_voltage() {
    std::lock_guard<std::mutex> lock(_mutex);
    return _battery.voltage;
}

// Competition functions
void HAL::set_robot_mode(RobotMode mode) {
    std::lock_guard<std::mutex> lock(_mutex);
    _robot_mode = mode;
}

RobotMode HAL::get_robot_mode() {
    std::lock_guard<std::mutex> lock(_mutex);
    return _robot_mode;
}

bool HAL::is_autonomous() {
    std::lock_guard<std::mutex> lock(_mutex);
    return _robot_mode == RobotMode::AUTONOMOUS;
}

bool HAL::is_disabled() {
    std::lock_guard<std::mutex> lock(_mutex);
    return _robot_mode == RobotMode::DISABLED;
}

bool HAL::is_connected() {
    std::lock_guard<std::mutex> lock(_mutex);
    return _competition_connected;
}

// LCD functions
void HAL::lcd_set_text(int16_t line, const std::string& text) {
    if (line < 0 || line > 7) return;
    std::lock_guard<std::mutex> lock(_mutex);
    _lcd_lines[line] = text;
    _lcd_initialized = true;
}

std::string HAL::lcd_get_text(int16_t line) {
    if (line < 0 || line > 7) return "";
    std::lock_guard<std::mutex> lock(_mutex);
    return _lcd_lines[line];
}

void HAL::lcd_clear() {
    std::lock_guard<std::mutex> lock(_mutex);
    for (auto& line : _lcd_lines) {
        line.clear();
    }
}

void HAL::lcd_clear_line(int16_t line) {
    if (line < 0 || line > 7) return;
    std::lock_guard<std::mutex> lock(_mutex);
    _lcd_lines[line].clear();
}

void HAL::lcd_set_button(uint8_t button, bool pressed) {
    std::lock_guard<std::mutex> lock(_mutex);
    if (pressed) {
        _lcd_buttons |= button;
    } else {
        _lcd_buttons &= ~button;
    }
}

uint8_t HAL::lcd_get_buttons() {
    std::lock_guard<std::mutex> lock(_mutex);
    return _lcd_buttons;
}

void HAL::lcd_set_background_color(uint32_t color) {
    std::lock_guard<std::mutex> lock(_mutex);
    _lcd_bg_color = color;
}

void HAL::lcd_set_text_color(uint32_t color) {
    std::lock_guard<std::mutex> lock(_mutex);
    _lcd_text_color = color;
}

uint32_t HAL::lcd_get_background_color() {
    std::lock_guard<std::mutex> lock(_mutex);
    return _lcd_bg_color;
}

uint32_t HAL::lcd_get_text_color() {
    std::lock_guard<std::mutex> lock(_mutex);
    return _lcd_text_color;
}

// State access for IPC
const MotorState& HAL::get_motor_state(uint8_t port) {
    static MotorState dummy;
    if (port < 1 || port > 21) return dummy;
    std::lock_guard<std::mutex> lock(_mutex);
    return _motors[port - 1];
}

const ControllerState& HAL::get_controller_state(pros::controller_id_e_t id) {
    static ControllerState dummy;
    if (id > 1) return dummy;
    std::lock_guard<std::mutex> lock(_mutex);
    return _controllers[id];
}

const BatteryState& HAL::get_battery_state() {
    std::lock_guard<std::mutex> lock(_mutex);
    return _battery;
}

void HAL::set_state_callback(StateCallback callback) {
    std::lock_guard<std::mutex> lock(_mutex);
    _state_callback = callback;
}

} // namespace host
