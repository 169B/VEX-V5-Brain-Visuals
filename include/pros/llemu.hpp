/**
 * @file llemu.hpp
 * @brief PROS Legacy LCD Emulator (LLEMU) API for Host Mode
 * 
 * This header provides the pros::lcd namespace functions that match the PROS API
 * for interacting with the VEX V5 Brain LCD.
 */

#ifndef PROS_LLEMU_HPP
#define PROS_LLEMU_HPP

#include <cstdint>
#include <string>
#include <functional>

namespace pros {
namespace lcd {

/**
 * LCD button enumeration
 */
typedef enum lcd_btn_e {
    LCD_BTN_LEFT = 4,
    LCD_BTN_CENTER = 2,
    LCD_BTN_RIGHT = 1
} lcd_btn_e_t;

/**
 * Callback type for LCD button presses
 */
using lcd_btn_cb_fn_t = void (*)();

/**
 * Initializes the LLEMU LCD.
 *
 * @return True if the LCD was successfully initialized
 */
bool initialize();

/**
 * Checks if the LLEMU LCD is initialized.
 *
 * @return True if the LCD is initialized
 */
bool is_initialized();

/**
 * Shuts down the LLEMU LCD.
 *
 * @return True if successful
 */
bool shutdown();

/**
 * Prints a formatted string to the LCD.
 *
 * @param line The line number (0-7)
 * @param fmt The format string
 * @return True if successful
 */
bool print(int16_t line, const char* fmt, ...);

/**
 * Sets the text on a line of the LCD.
 *
 * @param line The line number (0-7)
 * @param text The text to display
 * @return True if successful
 */
bool set_text(int16_t line, std::string text);

/**
 * Clears the LCD.
 *
 * @return True if successful
 */
bool clear();

/**
 * Clears a line on the LCD.
 *
 * @param line The line number (0-7)
 * @return True if successful
 */
bool clear_line(int16_t line);

/**
 * Registers a callback for the left button.
 *
 * @param cb The callback function
 */
void register_btn0_cb(lcd_btn_cb_fn_t cb);

/**
 * Registers a callback for the center button.
 *
 * @param cb The callback function
 */
void register_btn1_cb(lcd_btn_cb_fn_t cb);

/**
 * Registers a callback for the right button.
 *
 * @param cb The callback function
 */
void register_btn2_cb(lcd_btn_cb_fn_t cb);

/**
 * Reads the current button status.
 *
 * @return Bitmask of pressed buttons
 */
uint8_t read_buttons();

/**
 * Sets the background color.
 *
 * @param color The color (RGB565)
 */
void set_background_color(uint32_t color);

/**
 * Sets the text color.
 *
 * @param color The color (RGB565)
 */
void set_text_color(uint32_t color);

} // namespace lcd
} // namespace pros

// C-style function declarations
extern "C" {
    bool lcd_initialize();
    bool lcd_is_initialized();
    bool lcd_shutdown();
    bool lcd_print(int16_t line, const char* fmt, ...);
    bool lcd_set_text(int16_t line, const char* text);
    bool lcd_clear();
    bool lcd_clear_line(int16_t line);
    void lcd_register_btn0_cb(void (*cb)());
    void lcd_register_btn1_cb(void (*cb)());
    void lcd_register_btn2_cb(void (*cb)());
    uint8_t lcd_read_buttons();
}

#endif // PROS_LLEMU_HPP
