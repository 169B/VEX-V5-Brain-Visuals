/**
 * @file misc.hpp
 * @brief PROS Miscellaneous Functions for Host Mode
 * 
 * This header provides delay, millis, and other utility functions
 * that match the PROS API.
 */

#ifndef PROS_MISC_HPP
#define PROS_MISC_HPP

#include <cstdint>

namespace pros {

/**
 * Delay the current task for a given number of milliseconds.
 *
 * @param milliseconds The number of milliseconds to wait
 */
void delay(uint32_t milliseconds);

/**
 * Get the number of milliseconds since the program started.
 *
 * @return The number of milliseconds since program start
 */
uint32_t millis();

/**
 * Get the number of microseconds since the program started.
 *
 * @return The number of microseconds since program start
 */
uint64_t micros();

/**
 * Battery functions
 */
namespace battery {
    /**
     * Gets the current capacity of the battery.
     *
     * @return The battery capacity in percent (0-100)
     */
    double get_capacity();

    /**
     * Gets the current current draw from the battery.
     *
     * @return The battery current in mA
     */
    int32_t get_current();

    /**
     * Gets the temperature of the battery.
     *
     * @return The battery temperature in degrees Celsius
     */
    double get_temperature();

    /**
     * Gets the current voltage of the battery.
     *
     * @return The battery voltage in mV
     */
    int32_t get_voltage();
}

/**
 * Competition control functions
 */
namespace competition {
    /**
     * Gets the current competition status.
     *
     * @return The competition status bits
     */
    uint8_t get_status();

    /**
     * Checks if the robot is in autonomous mode.
     *
     * @return True if in autonomous mode
     */
    bool is_autonomous();

    /**
     * Checks if the robot is connected to competition control.
     *
     * @return True if connected
     */
    bool is_connected();

    /**
     * Checks if the robot is disabled.
     *
     * @return True if disabled
     */
    bool is_disabled();
}

} // namespace pros

// C-style function declarations for compatibility
extern "C" {
    void delay(uint32_t milliseconds);
    uint32_t millis();
    uint64_t micros();
}

#endif // PROS_MISC_HPP
