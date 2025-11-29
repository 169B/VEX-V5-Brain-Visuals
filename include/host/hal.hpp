/**
 * @file hal.hpp
 * @brief Hardware Abstraction Layer for Host Mode
 * 
 * This header provides the HAL singleton that manages motor state,
 * controller input, and other hardware simulation for host mode.
 */

#ifndef HOST_HAL_HPP
#define HOST_HAL_HPP

#include <cstdint>
#include <mutex>
#include <array>
#include <string>
#include <functional>
#include "pros/motors.hpp"
#include "pros/controller.hpp"

namespace host {

/**
 * Robot operation mode
 */
enum class RobotMode {
    DISABLED,
    AUTONOMOUS,
    OPCONTROL
};

/**
 * Motor state structure
 */
struct MotorState {
    int32_t voltage = 0;           // Current voltage (-127 to 127)
    int32_t velocity = 0;          // Target velocity
    double position = 0.0;         // Current position in encoder units
    double actual_velocity = 0.0;  // Actual velocity
    int32_t current = 0;           // Current draw in mA
    double temperature = 25.0;     // Temperature in Celsius
    pros::motor_gearset_e_t gearset = pros::E_MOTOR_GEARSET_18;
    bool reversed = false;
    bool connected = false;
};

/**
 * Controller state structure
 */
struct ControllerState {
    std::array<int32_t, 4> analog = {0, 0, 0, 0};  // LX, LY, RX, RY
    std::array<bool, 18> digital = {false};        // All buttons
    bool connected = false;
    int32_t battery_capacity = 100;
    int32_t battery_level = 100;
    std::array<std::string, 3> lcd_lines;          // Controller LCD
};

/**
 * Battery state structure
 */
struct BatteryState {
    double capacity = 100.0;       // Percent (0-100)
    int32_t current = 0;           // mA
    double temperature = 25.0;     // Celsius
    int32_t voltage = 12600;       // mV
};

/**
 * Hardware Abstraction Layer singleton
 */
class HAL {
public:
    /**
     * Gets the singleton instance.
     *
     * @return Reference to the HAL instance
     */
    static HAL& instance();

    // Delete copy/move constructors
    HAL(const HAL&) = delete;
    HAL& operator=(const HAL&) = delete;
    HAL(HAL&&) = delete;
    HAL& operator=(HAL&&) = delete;

    /**
     * Initializes the HAL.
     */
    void init();

    /**
     * Shuts down the HAL.
     */
    void shutdown();

    /**
     * Updates the HAL state (called periodically).
     */
    void update();

    // Motor functions
    void set_motor(uint8_t port, int32_t voltage);
    void set_motor_velocity(uint8_t port, int32_t velocity);
    void set_motor_position(uint8_t port, double position);
    void set_motor_gearset(uint8_t port, pros::motor_gearset_e_t gearset);
    void set_motor_reversed(uint8_t port, bool reversed);
    void set_motor_connected(uint8_t port, bool connected);
    
    int32_t get_motor_voltage(uint8_t port);
    int32_t get_motor_velocity(uint8_t port);
    double get_motor_position(uint8_t port);
    double get_motor_actual_velocity(uint8_t port);
    int32_t get_motor_current(uint8_t port);
    double get_motor_temperature(uint8_t port);
    pros::motor_gearset_e_t get_motor_gearset(uint8_t port);
    bool get_motor_reversed(uint8_t port);
    bool is_motor_connected(uint8_t port);

    // Controller functions
    void set_controller_analog(pros::controller_id_e_t id, pros::controller_analog_e_t channel, int32_t value);
    void set_controller_digital(pros::controller_id_e_t id, pros::controller_digital_e_t button, bool value);
    void set_controller_connected(pros::controller_id_e_t id, bool connected);
    
    int32_t get_controller_analog(pros::controller_id_e_t id, pros::controller_analog_e_t channel);
    bool get_controller_digital(pros::controller_id_e_t id, pros::controller_digital_e_t button);
    bool is_controller_connected(pros::controller_id_e_t id);
    int32_t get_controller_battery_capacity(pros::controller_id_e_t id);
    int32_t get_controller_battery_level(pros::controller_id_e_t id);

    // Battery functions
    double get_battery_capacity();
    int32_t get_battery_current();
    double get_battery_temperature();
    int32_t get_battery_voltage();

    // Competition functions
    void set_robot_mode(RobotMode mode);
    RobotMode get_robot_mode();
    bool is_autonomous();
    bool is_disabled();
    bool is_connected();

    // LCD functions (LLEMU)
    void lcd_set_text(int16_t line, const std::string& text);
    std::string lcd_get_text(int16_t line);
    void lcd_clear();
    void lcd_clear_line(int16_t line);
    void lcd_set_button(uint8_t button, bool pressed);
    uint8_t lcd_get_buttons();
    void lcd_set_background_color(uint32_t color);
    void lcd_set_text_color(uint32_t color);
    uint32_t lcd_get_background_color();
    uint32_t lcd_get_text_color();

    // State access for IPC
    const MotorState& get_motor_state(uint8_t port);
    const ControllerState& get_controller_state(pros::controller_id_e_t id);
    const BatteryState& get_battery_state();

    // Callback registration
    using StateCallback = std::function<void()>;
    void set_state_callback(StateCallback callback);

private:
    HAL();
    ~HAL();

    std::mutex _mutex;
    
    // Motor states (ports 1-21)
    std::array<MotorState, 21> _motors;
    
    // Controller states
    std::array<ControllerState, 2> _controllers;
    
    // Battery state
    BatteryState _battery;
    
    // Competition state
    RobotMode _robot_mode = RobotMode::DISABLED;
    bool _competition_connected = false;
    
    // LCD state
    std::array<std::string, 8> _lcd_lines;
    uint8_t _lcd_buttons = 0;
    uint32_t _lcd_bg_color = 0x0000;  // Black
    uint32_t _lcd_text_color = 0xFFFF; // White
    bool _lcd_initialized = false;
    
    // Callback for state changes
    StateCallback _state_callback;
};

} // namespace host

#endif // HOST_HAL_HPP
