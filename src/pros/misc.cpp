/**
 * @file misc.cpp
 * @brief PROS Miscellaneous Functions Implementation for Host Mode
 */

#include "pros/misc.hpp"
#include "host/hal.hpp"
#include <chrono>
#include <thread>

// Static start time for timing functions
static auto program_start = std::chrono::steady_clock::now();

namespace pros {

void delay(uint32_t milliseconds) {
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

uint32_t millis() {
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - program_start);
    return static_cast<uint32_t>(duration.count());
}

uint64_t micros() {
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(now - program_start);
    return static_cast<uint64_t>(duration.count());
}

namespace battery {

double get_capacity() {
    return host::HAL::instance().get_battery_capacity();
}

int32_t get_current() {
    return host::HAL::instance().get_battery_current();
}

double get_temperature() {
    return host::HAL::instance().get_battery_temperature();
}

int32_t get_voltage() {
    return host::HAL::instance().get_battery_voltage();
}

} // namespace battery

namespace competition {

static uint8_t competition_status = 0;

uint8_t get_status() {
    return competition_status;
}

bool is_autonomous() {
    return host::HAL::instance().is_autonomous();
}

bool is_connected() {
    return host::HAL::instance().is_connected();
}

bool is_disabled() {
    return host::HAL::instance().is_disabled();
}

} // namespace competition

} // namespace pros

// C-style functions
extern "C" {

void delay(uint32_t milliseconds) {
    pros::delay(milliseconds);
}

uint32_t millis() {
    return pros::millis();
}

uint64_t micros() {
    return pros::micros();
}

} // extern "C"
