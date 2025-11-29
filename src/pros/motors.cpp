/**
 * @file motors.cpp
 * @brief PROS Motor Implementation for Host Mode
 */

#include "pros/motors.hpp"
#include "host/hal.hpp"
#include <algorithm>
#include <cmath>

namespace pros {

Motor::Motor(int8_t port, motor_gearset_e_t gearset, bool reverse)
    : _port(port), _gearset(gearset), _reversed(reverse),
      _brake_mode(E_MOTOR_BRAKE_COAST),
      _encoder_units(E_MOTOR_ENCODER_DEGREES),
      _current_limit(2500),
      _voltage_limit(12000),
      _zero_position(0.0) {
    
    // Handle negative port (indicates reversed)
    if (port < 0) {
        _port = -port;
        _reversed = !reverse;
    }
    
    // Clamp port to valid range
    if (_port < 1) _port = 1;
    if (_port > 21) _port = 21;
    
    // Initialize motor in HAL
    host::HAL::instance().set_motor_gearset(_port, _gearset);
    host::HAL::instance().set_motor_reversed(_port, _reversed);
    host::HAL::instance().set_motor_connected(_port, true);
}

Motor::Motor(int8_t port, bool reverse)
    : Motor(port, E_MOTOR_GEARSET_18, reverse) {}

int32_t Motor::move(int32_t voltage) {
    // Clamp voltage to -127 to 127
    voltage = std::clamp(voltage, -127, 127);
    
    // Apply reversal
    if (_reversed) {
        voltage = -voltage;
    }
    
    host::HAL::instance().set_motor(_port, voltage);
    return 1;
}

int32_t Motor::move_absolute(double position, int32_t velocity) {
    // Calculate direction based on current position
    double current = get_position();
    double diff = position - current;
    
    if (std::abs(diff) < 0.1) {
        // Already at position
        host::HAL::instance().set_motor_velocity(_port, 0);
        return 1;
    }
    
    // Set velocity in appropriate direction
    int32_t vel = (diff > 0) ? std::abs(velocity) : -std::abs(velocity);
    if (_reversed) vel = -vel;
    
    host::HAL::instance().set_motor_velocity(_port, vel);
    host::HAL::instance().set_motor_position(_port, position);
    return 1;
}

int32_t Motor::move_relative(double position, int32_t velocity) {
    double current = get_position();
    return move_absolute(current + position, velocity);
}

int32_t Motor::move_velocity(int32_t velocity) {
    // Apply reversal
    if (_reversed) {
        velocity = -velocity;
    }
    
    host::HAL::instance().set_motor_velocity(_port, velocity);
    return 1;
}

int32_t Motor::move_voltage(int32_t voltage) {
    // Clamp to -12000 to 12000 mV
    voltage = std::clamp(voltage, -12000, 12000);
    
    // Convert to -127 to 127 scale
    int32_t scaled = (voltage * 127) / 12000;
    
    return move(scaled);
}

motor_brake_mode_e_t Motor::get_brake_mode() {
    return _brake_mode;
}

int32_t Motor::get_current_limit() {
    return _current_limit;
}

int32_t Motor::get_current_draw() {
    return host::HAL::instance().get_motor_current(_port);
}

int32_t Motor::get_direction() {
    double vel = get_actual_velocity();
    if (vel > 0.1) return 1;
    if (vel < -0.1) return -1;
    return 0;
}

double Motor::get_efficiency() {
    // Simulated efficiency based on load
    return 80.0;
}

motor_gearset_e_t Motor::get_gearing() {
    return host::HAL::instance().get_motor_gearset(_port);
}

motor_encoder_units_e_t Motor::get_encoder_units() {
    return _encoder_units;
}

double Motor::get_position() {
    double pos = host::HAL::instance().get_motor_position(_port);
    return pos - _zero_position;
}

double Motor::get_power() {
    // P = V * I (simplified)
    double voltage = host::HAL::instance().get_motor_voltage(_port) / 127.0 * 12.0;
    double current = get_current_draw() / 1000.0;
    return voltage * current;
}

double Motor::get_target_position() {
    return host::HAL::instance().get_motor_position(_port);
}

int32_t Motor::get_target_velocity() {
    return host::HAL::instance().get_motor_velocity(_port);
}

double Motor::get_temperature() {
    return host::HAL::instance().get_motor_temperature(_port);
}

double Motor::get_torque() {
    // Simplified torque calculation
    double current = get_current_draw() / 1000.0;
    return current * 0.01; // Nm per A (approximate)
}

double Motor::get_actual_velocity() {
    double vel = host::HAL::instance().get_motor_actual_velocity(_port);
    if (_reversed) vel = -vel;
    return vel;
}

int32_t Motor::get_voltage() {
    int32_t voltage = host::HAL::instance().get_motor_voltage(_port);
    // Convert from -127..127 to mV
    return (voltage * 12000) / 127;
}

int32_t Motor::get_voltage_limit() {
    return _voltage_limit;
}

bool Motor::is_stopped() {
    return std::abs(get_actual_velocity()) < 0.1;
}

bool Motor::is_over_current() {
    return get_current_draw() > _current_limit;
}

bool Motor::is_over_temp() {
    return get_temperature() > 55.0;
}

int8_t Motor::get_port() {
    return _port;
}

int32_t Motor::set_brake_mode(motor_brake_mode_e_t mode) {
    _brake_mode = mode;
    return 1;
}

int32_t Motor::set_current_limit(int32_t limit) {
    _current_limit = limit;
    return 1;
}

int32_t Motor::set_encoder_units(motor_encoder_units_e_t units) {
    _encoder_units = units;
    return 1;
}

int32_t Motor::set_gearing(motor_gearset_e_t gearset) {
    _gearset = gearset;
    host::HAL::instance().set_motor_gearset(_port, gearset);
    return 1;
}

int32_t Motor::set_reversed(bool reverse) {
    _reversed = reverse;
    host::HAL::instance().set_motor_reversed(_port, reverse);
    return 1;
}

int32_t Motor::set_voltage_limit(int32_t limit) {
    _voltage_limit = std::clamp(limit, 0, 12000);
    return 1;
}

int32_t Motor::set_zero_position(double position) {
    _zero_position = position;
    return 1;
}

int32_t Motor::tare_position() {
    _zero_position = host::HAL::instance().get_motor_position(_port);
    return 1;
}

} // namespace pros
