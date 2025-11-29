/**
 * @file controller.hpp
 * @brief PROS Controller API for Host Mode
 * 
 * This header provides the pros::Controller class that matches the PROS API
 * for reading controller input.
 */

#ifndef PROS_CONTROLLER_HPP
#define PROS_CONTROLLER_HPP

#include <cstdint>
#include <string>

namespace pros {

/**
 * Controller ID enumeration
 */
typedef enum controller_id_e {
    E_CONTROLLER_MASTER = 0,
    E_CONTROLLER_PARTNER = 1
} controller_id_e_t;

/**
 * Controller analog channel enumeration
 */
typedef enum controller_analog_e {
    E_CONTROLLER_ANALOG_LEFT_X = 0,
    E_CONTROLLER_ANALOG_LEFT_Y = 1,
    E_CONTROLLER_ANALOG_RIGHT_X = 2,
    E_CONTROLLER_ANALOG_RIGHT_Y = 3
} controller_analog_e_t;

/**
 * Controller digital button enumeration
 */
typedef enum controller_digital_e {
    E_CONTROLLER_DIGITAL_L1 = 6,
    E_CONTROLLER_DIGITAL_L2 = 7,
    E_CONTROLLER_DIGITAL_R1 = 8,
    E_CONTROLLER_DIGITAL_R2 = 9,
    E_CONTROLLER_DIGITAL_UP = 10,
    E_CONTROLLER_DIGITAL_DOWN = 11,
    E_CONTROLLER_DIGITAL_LEFT = 12,
    E_CONTROLLER_DIGITAL_RIGHT = 13,
    E_CONTROLLER_DIGITAL_X = 14,
    E_CONTROLLER_DIGITAL_B = 15,
    E_CONTROLLER_DIGITAL_Y = 16,
    E_CONTROLLER_DIGITAL_A = 17
} controller_digital_e_t;

/**
 * Controller class for reading VEX V5 controller input
 */
class Controller {
public:
    /**
     * Creates a Controller object for the given controller ID.
     *
     * @param id The controller ID (master or partner)
     */
    Controller(controller_id_e_t id);

    /**
     * Checks if the controller is connected.
     *
     * @return True if the controller is connected
     */
    bool is_connected();

    /**
     * Gets the value of an analog channel on the controller.
     *
     * @param channel The analog channel to read
     * @return The analog channel value from -127 to 127
     */
    int32_t get_analog(controller_analog_e_t channel);

    /**
     * Gets the value of a digital button on the controller.
     *
     * @param button The digital button to read
     * @return True if the button is pressed
     */
    int32_t get_digital(controller_digital_e_t button);

    /**
     * Gets whether a digital button has been pressed since last call.
     *
     * @param button The digital button to check
     * @return True if the button was newly pressed
     */
    int32_t get_digital_new_press(controller_digital_e_t button);

    /**
     * Gets the battery capacity of the controller.
     *
     * @return The battery capacity (0-100)
     */
    int32_t get_battery_capacity();

    /**
     * Gets the battery level of the controller.
     *
     * @return The battery level (0-100)
     */
    int32_t get_battery_level();

    /**
     * Clears a line on the controller LCD.
     *
     * @param line The line number (0-2)
     * @return 1 if successful
     */
    int32_t clear_line(uint8_t line);

    /**
     * Clears the controller LCD.
     *
     * @return 1 if successful
     */
    int32_t clear();

    /**
     * Prints text to the controller LCD.
     *
     * @param line The line number (0-2)
     * @param col The column to start at (0-14)
     * @param fmt The format string
     * @return 1 if successful
     */
    int32_t print(uint8_t line, uint8_t col, const char* fmt, ...);

    /**
     * Sets text on the controller LCD.
     *
     * @param line The line number (0-2)
     * @param col The column to start at (0-14)
     * @param str The string to display
     * @return 1 if successful
     */
    int32_t set_text(uint8_t line, uint8_t col, const char* str);

    /**
     * Rumbles the controller.
     *
     * @param rumble_pattern A string of '-' and '.' characters
     * @return 1 if successful
     */
    int32_t rumble(const char* rumble_pattern);

private:
    controller_id_e_t _id;
    int32_t _last_digital_state[18]; // For tracking new presses
};

} // namespace pros

#endif // PROS_CONTROLLER_HPP
