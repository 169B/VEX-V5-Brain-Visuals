/**
 * @file motors.hpp
 * @brief PROS Motor API for Host Mode
 * 
 * This header provides the pros::Motor class that matches the PROS API
 * for compatibility with VEX V5 robot code.
 */

#ifndef PROS_MOTORS_HPP
#define PROS_MOTORS_HPP

#include <cstdint>

namespace pros {

/**
 * Motor gearset enumeration
 */
typedef enum motor_gearset_e {
    E_MOTOR_GEARSET_36 = 0,  // 36:1 (100 RPM)
    E_MOTOR_GEARSET_18 = 1,  // 18:1 (200 RPM) - default
    E_MOTOR_GEARSET_06 = 2,  // 6:1 (600 RPM)
    E_MOTOR_GEARSET_INVALID = INT32_MAX
} motor_gearset_e_t;

/**
 * Motor brake mode enumeration
 */
typedef enum motor_brake_mode_e {
    E_MOTOR_BRAKE_COAST = 0,
    E_MOTOR_BRAKE_BRAKE = 1,
    E_MOTOR_BRAKE_HOLD = 2,
    E_MOTOR_BRAKE_INVALID = INT32_MAX
} motor_brake_mode_e_t;

/**
 * Motor encoder units enumeration
 */
typedef enum motor_encoder_units_e {
    E_MOTOR_ENCODER_DEGREES = 0,
    E_MOTOR_ENCODER_ROTATIONS = 1,
    E_MOTOR_ENCODER_COUNTS = 2,
    E_MOTOR_ENCODER_INVALID = INT32_MAX
} motor_encoder_units_e_t;

/**
 * Motor class for controlling VEX V5 Smart Motors
 */
class Motor {
public:
    /**
     * Creates a Motor object for the given port.
     *
     * @param port The V5 port number from 1-21
     * @param gearset The motor's gearset
     * @param reverse Whether the motor direction is reversed
     */
    Motor(int8_t port, motor_gearset_e_t gearset = E_MOTOR_GEARSET_18, bool reverse = false);

    /**
     * Creates a Motor object for the given port with default gearset.
     *
     * @param port The V5 port number from 1-21
     * @param reverse Whether the motor direction is reversed
     */
    Motor(int8_t port, bool reverse);

    /**
     * Sets the voltage for the motor from -127 to 127.
     *
     * @param voltage The new motor voltage from -127 to 127
     * @return 1 if the operation was successful or PROS_ERR if the operation failed
     */
    int32_t move(int32_t voltage);

    /**
     * Sets the target absolute position for the motor.
     *
     * @param position The absolute position to move to
     * @param velocity The maximum velocity to use
     * @return 1 if the operation was successful or PROS_ERR if the operation failed
     */
    int32_t move_absolute(double position, int32_t velocity);

    /**
     * Sets the relative target position for the motor.
     *
     * @param position The relative position to move to
     * @param velocity The maximum velocity to use
     * @return 1 if the operation was successful or PROS_ERR if the operation failed
     */
    int32_t move_relative(double position, int32_t velocity);

    /**
     * Sets the velocity for the motor.
     *
     * @param velocity The new motor velocity from -MAX to MAX
     * @return 1 if the operation was successful or PROS_ERR if the operation failed
     */
    int32_t move_velocity(int32_t velocity);

    /**
     * Sets the output voltage for the motor from -12000 to 12000.
     *
     * @param voltage The new voltage value from -12000 to 12000
     * @return 1 if the operation was successful or PROS_ERR if the operation failed
     */
    int32_t move_voltage(int32_t voltage);

    /**
     * Gets the brake mode of the motor.
     *
     * @return The brake mode of the motor
     */
    motor_brake_mode_e_t get_brake_mode();

    /**
     * Gets the current limit for the motor.
     *
     * @return The motor's current limit in mA
     */
    int32_t get_current_limit();

    /**
     * Gets the currently drawn current for the motor.
     *
     * @return The motor's current in mA
     */
    int32_t get_current_draw();

    /**
     * Gets the direction of movement for the motor.
     *
     * @return 1 for moving in the positive direction, -1 for moving in the negative direction, or PROS_ERR if failed
     */
    int32_t get_direction();

    /**
     * Gets the efficiency of the motor in percent.
     *
     * @return The motor's efficiency in percent or PROS_ERR_F if the operation failed
     */
    double get_efficiency();

    /**
     * Gets the gearset of the motor.
     *
     * @return The motor's gearset
     */
    motor_gearset_e_t get_gearing();

    /**
     * Gets the encoder units of the motor.
     *
     * @return The motor's encoder units
     */
    motor_encoder_units_e_t get_encoder_units();

    /**
     * Gets the absolute position of the motor.
     *
     * @return The motor's absolute position
     */
    double get_position();

    /**
     * Gets the power drawn by the motor in watts.
     *
     * @return The motor's power consumption in watts
     */
    double get_power();

    /**
     * Gets the target position for the motor.
     *
     * @return The motor's target position
     */
    double get_target_position();

    /**
     * Gets the target velocity for the motor.
     *
     * @return The motor's target velocity
     */
    int32_t get_target_velocity();

    /**
     * Gets the temperature of the motor in degrees Celsius.
     *
     * @return The motor's temperature in degrees Celsius
     */
    double get_temperature();

    /**
     * Gets the torque generated by the motor in Nm.
     *
     * @return The motor's torque in Nm
     */
    double get_torque();

    /**
     * Gets the actual velocity of the motor.
     *
     * @return The motor's actual velocity
     */
    double get_actual_velocity();

    /**
     * Gets the voltage delivered to the motor in millivolts.
     *
     * @return The motor's voltage in mV
     */
    int32_t get_voltage();

    /**
     * Gets the voltage limit for the motor.
     *
     * @return The motor's voltage limit in mV
     */
    int32_t get_voltage_limit();

    /**
     * Checks if the motor is at its zero position.
     *
     * @return True if the motor is at zero position
     */
    bool is_stopped();

    /**
     * Checks if the motor is over its current limit.
     *
     * @return True if the motor is over current
     */
    bool is_over_current();

    /**
     * Checks if the motor is over temperature.
     *
     * @return True if the motor is over temp
     */
    bool is_over_temp();

    /**
     * Gets the port number of the motor.
     *
     * @return The motor's port number
     */
    int8_t get_port();

    /**
     * Sets the brake mode for the motor.
     *
     * @param mode The new brake mode
     * @return 1 if operation was successful
     */
    int32_t set_brake_mode(motor_brake_mode_e_t mode);

    /**
     * Sets the current limit for the motor.
     *
     * @param limit The new current limit in mA
     * @return 1 if operation was successful
     */
    int32_t set_current_limit(int32_t limit);

    /**
     * Sets the encoder units for the motor.
     *
     * @param units The new encoder units
     * @return 1 if operation was successful
     */
    int32_t set_encoder_units(motor_encoder_units_e_t units);

    /**
     * Sets the gearset for the motor.
     *
     * @param gearset The new gearset
     * @return 1 if operation was successful
     */
    int32_t set_gearing(motor_gearset_e_t gearset);

    /**
     * Sets the reversed flag for the motor.
     *
     * @param reverse True to reverse motor direction
     * @return 1 if operation was successful
     */
    int32_t set_reversed(bool reverse);

    /**
     * Sets the voltage limit for the motor.
     *
     * @param limit The new voltage limit in mV
     * @return 1 if operation was successful
     */
    int32_t set_voltage_limit(int32_t limit);

    /**
     * Sets the zero position for the motor.
     *
     * @param position The new zero position
     * @return 1 if operation was successful
     */
    int32_t set_zero_position(double position);

    /**
     * Resets the motor's encoder to zero.
     *
     * @return 1 if operation was successful
     */
    int32_t tare_position();

private:
    int8_t _port;
    motor_gearset_e_t _gearset;
    bool _reversed;
    motor_brake_mode_e_t _brake_mode;
    motor_encoder_units_e_t _encoder_units;
    int32_t _current_limit;
    int32_t _voltage_limit;
    double _zero_position;
};

} // namespace pros

#endif // PROS_MOTORS_HPP
