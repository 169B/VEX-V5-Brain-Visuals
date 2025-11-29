/**
 * @file controller.cpp
 * @brief PROS Controller Implementation for Host Mode
 */

#include "pros/controller.hpp"
#include "host/hal.hpp"
#include <cstdarg>
#include <cstdio>
#include <cstring>

#ifndef PROS_ERR
#define PROS_ERR (-1)
#endif

namespace pros {

Controller::Controller(controller_id_e_t id) : _id(id) {
    // Initialize last digital state
    for (int i = 0; i < 18; i++) {
        _last_digital_state[i] = 0;
    }
}

bool Controller::is_connected() {
    return host::HAL::instance().is_controller_connected(_id);
}

int32_t Controller::get_analog(controller_analog_e_t channel) {
    return host::HAL::instance().get_controller_analog(_id, channel);
}

int32_t Controller::get_digital(controller_digital_e_t button) {
    return host::HAL::instance().get_controller_digital(_id, button) ? 1 : 0;
}

int32_t Controller::get_digital_new_press(controller_digital_e_t button) {
    int button_idx = static_cast<int>(button);
    if (button_idx < 0 || button_idx >= 18) return 0;
    
    int32_t current = get_digital(button);
    int32_t last = _last_digital_state[button_idx];
    _last_digital_state[button_idx] = current;
    
    // New press is when current is pressed and last was not
    return (current && !last) ? 1 : 0;
}

int32_t Controller::get_battery_capacity() {
    return host::HAL::instance().get_controller_battery_capacity(_id);
}

int32_t Controller::get_battery_level() {
    return host::HAL::instance().get_controller_battery_level(_id);
}

int32_t Controller::clear_line(uint8_t line) {
    if (line > 2) return PROS_ERR;
    // In host mode, we don't actually have controller LCD
    // This is just a stub
    return 1;
}

int32_t Controller::clear() {
    for (uint8_t i = 0; i < 3; i++) {
        clear_line(i);
    }
    return 1;
}

int32_t Controller::print(uint8_t line, uint8_t col, const char* fmt, ...) {
    if (line > 2 || col > 14) return PROS_ERR;
    
    char buffer[32];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    
    // In host mode, we could send this to the UI
    // For now, it's just a stub
    return 1;
}

int32_t Controller::set_text(uint8_t line, uint8_t col, const char* /*str*/) {
    if (line > 2 || col > 14) return PROS_ERR;
    // Stub implementation
    return 1;
}

int32_t Controller::rumble(const char* rumble_pattern) {
    // In host mode, we could send haptic feedback to UI
    // For now, just validate the pattern
    if (!rumble_pattern) return PROS_ERR;
    
    // Pattern should only contain '-' (long), '.' (short), and ' ' (pause)
    for (const char* p = rumble_pattern; *p; p++) {
        if (*p != '-' && *p != '.' && *p != ' ') {
            return PROS_ERR;
        }
    }
    
    return 1;
}

} // namespace pros
