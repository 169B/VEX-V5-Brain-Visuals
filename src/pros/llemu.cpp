/**
 * @file llemu.cpp
 * @brief PROS Legacy LCD Emulator Implementation for Host Mode
 */

#include "pros/llemu.hpp"
#include "host/hal.hpp"
#include <cstdarg>
#include <cstdio>
#include <mutex>

// Static state
static bool lcd_initialized = false;
static std::mutex lcd_mutex;
static void (*btn0_callback)() = nullptr;
static void (*btn1_callback)() = nullptr;
static void (*btn2_callback)() = nullptr;

namespace pros {
namespace lcd {

bool initialize() {
    std::lock_guard<std::mutex> lock(lcd_mutex);
    lcd_initialized = true;
    host::HAL::instance().lcd_clear();
    return true;
}

bool is_initialized() {
    return lcd_initialized;
}

bool shutdown() {
    std::lock_guard<std::mutex> lock(lcd_mutex);
    lcd_initialized = false;
    btn0_callback = nullptr;
    btn1_callback = nullptr;
    btn2_callback = nullptr;
    return true;
}

bool print(int16_t line, const char* fmt, ...) {
    if (!lcd_initialized || line < 0 || line > 7) return false;
    
    char buffer[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    
    std::lock_guard<std::mutex> lock(lcd_mutex);
    host::HAL::instance().lcd_set_text(line, buffer);
    return true;
}

bool set_text(int16_t line, std::string text) {
    if (!lcd_initialized || line < 0 || line > 7) return false;
    
    std::lock_guard<std::mutex> lock(lcd_mutex);
    host::HAL::instance().lcd_set_text(line, text);
    return true;
}

bool clear() {
    if (!lcd_initialized) return false;
    
    std::lock_guard<std::mutex> lock(lcd_mutex);
    host::HAL::instance().lcd_clear();
    return true;
}

bool clear_line(int16_t line) {
    if (!lcd_initialized || line < 0 || line > 7) return false;
    
    std::lock_guard<std::mutex> lock(lcd_mutex);
    host::HAL::instance().lcd_clear_line(line);
    return true;
}

void register_btn0_cb(lcd_btn_cb_fn_t cb) {
    std::lock_guard<std::mutex> lock(lcd_mutex);
    btn0_callback = cb;
}

void register_btn1_cb(lcd_btn_cb_fn_t cb) {
    std::lock_guard<std::mutex> lock(lcd_mutex);
    btn1_callback = cb;
}

void register_btn2_cb(lcd_btn_cb_fn_t cb) {
    std::lock_guard<std::mutex> lock(lcd_mutex);
    btn2_callback = cb;
}

uint8_t read_buttons() {
    if (!lcd_initialized) return 0;
    return host::HAL::instance().lcd_get_buttons();
}

void set_background_color(uint32_t color) {
    std::lock_guard<std::mutex> lock(lcd_mutex);
    host::HAL::instance().lcd_set_background_color(color);
}

void set_text_color(uint32_t color) {
    std::lock_guard<std::mutex> lock(lcd_mutex);
    host::HAL::instance().lcd_set_text_color(color);
}

} // namespace lcd
} // namespace pros

// Button callback dispatcher (called from HAL)
void lcd_check_buttons() {
    uint8_t buttons = pros::lcd::read_buttons();
    static uint8_t last_buttons = 0;
    
    // Check for new presses
    uint8_t new_presses = buttons & ~last_buttons;
    
    if ((new_presses & pros::lcd::LCD_BTN_LEFT) && btn0_callback) {
        btn0_callback();
    }
    if ((new_presses & pros::lcd::LCD_BTN_CENTER) && btn1_callback) {
        btn1_callback();
    }
    if ((new_presses & pros::lcd::LCD_BTN_RIGHT) && btn2_callback) {
        btn2_callback();
    }
    
    last_buttons = buttons;
}

// C-style functions
extern "C" {

bool lcd_initialize() {
    return pros::lcd::initialize();
}

bool lcd_is_initialized() {
    return pros::lcd::is_initialized();
}

bool lcd_shutdown() {
    return pros::lcd::shutdown();
}

bool lcd_print(int16_t line, const char* fmt, ...) {
    if (!lcd_initialized || line < 0 || line > 7) return false;
    
    char buffer[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    
    return pros::lcd::set_text(line, buffer);
}

bool lcd_set_text(int16_t line, const char* text) {
    return pros::lcd::set_text(line, text ? text : "");
}

bool lcd_clear() {
    return pros::lcd::clear();
}

bool lcd_clear_line(int16_t line) {
    return pros::lcd::clear_line(line);
}

void lcd_register_btn0_cb(void (*cb)()) {
    pros::lcd::register_btn0_cb(cb);
}

void lcd_register_btn1_cb(void (*cb)()) {
    pros::lcd::register_btn1_cb(cb);
}

void lcd_register_btn2_cb(void (*cb)()) {
    pros::lcd::register_btn2_cb(cb);
}

uint8_t lcd_read_buttons() {
    return pros::lcd::read_buttons();
}

} // extern "C"
