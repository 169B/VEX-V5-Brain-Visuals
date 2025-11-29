/**
 * @file api.h
 * @brief Main PROS-like API Header for Host Mode
 * 
 * This header includes all the necessary PROS API components
 * for building VEX V5 robot code in host mode.
 */

#ifndef API_H
#define API_H

#ifdef __cplusplus

#include "pros/motors.hpp"
#include "pros/misc.hpp"
#include "pros/rtos.hpp"
#include "pros/controller.hpp"
#include "pros/llemu.hpp"
#include "liblvgl/lvgl.h"

/**
 * PROS error value
 */
#define PROS_ERR (-1)
#define PROS_ERR_F (-1.0)

/**
 * PROS version information
 */
#define PROS_VERSION_MAJOR 3
#define PROS_VERSION_MINOR 8
#define PROS_VERSION_PATCH 0
#define PROS_VERSION_STRING "3.8.0-host"

/**
 * User-defined functions that must be implemented
 */

/**
 * Runs initialization code. This occurs as soon as the program is started.
 *
 * All other competition modes are blocked by initialize; it is recommended
 * to keep execution time for this mode under a few seconds.
 */
void initialize();

/**
 * Runs while the robot is in the disabled state of Field Management System or
 * the VEX Competition Switch, following either autonomous or opcontrol. When
 * the robot is enabled, this task will exit.
 */
void disabled();

/**
 * Runs after initialize(), and before autonomous when connected to the Field
 * Management System or the VEX Competition Switch. This is intended for
 * competition-specific initialization routines, such as an autonomous selector
 * on the LCD.
 */
void competition_initialize();

/**
 * Runs the user autonomous code. This function will be started in its own task
 * with the default priority and stack size whenever the robot is enabled via
 * the Field Management System or the VEX Competition Switch in the autonomous
 * mode. Alternatively, this function may be called in initialize or opcontrol
 * for non-competition testing purposes.
 */
void autonomous();

/**
 * Runs the operator control code. This function will be started in its own task
 * with the default priority and stack size whenever the robot is enabled via
 * the Field Management System or the VEX Competition Switch in the operator
 * control mode. Alternatively, this function may be called in initialize or
 * opcontrol for non-competition testing purposes.
 */
void opcontrol();

#else
// C compatibility
#include <stdint.h>
#include <stdbool.h>
#include "liblvgl/lvgl.h"

#define PROS_ERR (-1)
#define PROS_ERR_F (-1.0)

void delay(uint32_t milliseconds);
uint32_t millis(void);
uint64_t micros(void);

void initialize(void);
void disabled(void);
void competition_initialize(void);
void autonomous(void);
void opcontrol(void);

#endif

#endif // API_H
