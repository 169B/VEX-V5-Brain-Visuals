/**
 * @file display.hpp
 * @brief LVGL Display Driver for Host Mode
 * 
 * This header provides the display and input driver initialization
 * for LVGL in host mode, routing display output to the IPC client.
 */

#ifndef HOST_DISPLAY_HPP
#define HOST_DISPLAY_HPP

#include "liblvgl/lvgl.h"
#include <cstdint>
#include <atomic>

namespace host {

/**
 * Display driver for host mode
 * 
 * Implements the LVGL 8.3 display driver interface, sending
 * framebuffer data to the UI via IPC.
 */
class Display {
public:
    /**
     * Gets the singleton instance.
     *
     * @return Reference to the Display instance
     */
    static Display& instance();

    // Delete copy/move constructors
    Display(const Display&) = delete;
    Display& operator=(const Display&) = delete;
    Display(Display&&) = delete;
    Display& operator=(Display&&) = delete;

    /**
     * Initializes LVGL and the display driver.
     * Must be called before using LVGL functions.
     */
    void init();

    /**
     * Shuts down the display driver.
     */
    void shutdown();

    /**
     * Checks if the display is initialized.
     *
     * @return True if initialized
     */
    bool is_initialized();

    /**
     * Updates touch input state from IPC.
     *
     * @param x Touch X coordinate
     * @param y Touch Y coordinate
     * @param pressed Whether touch is pressed
     */
    void set_touch(int16_t x, int16_t y, bool pressed);

    /**
     * Handles LVGL timer tasks.
     * Should be called periodically (e.g., every 5-10ms).
     */
    void update();

    /**
     * Gets a pointer to the framebuffer.
     *
     * @return Pointer to the framebuffer (480x272 RGB565)
     */
    const uint16_t* get_framebuffer();

    /**
     * Screen dimensions
     */
    static constexpr int WIDTH = 480;
    static constexpr int HEIGHT = 272;
    static constexpr int BUFFER_SIZE = WIDTH * HEIGHT;

private:
    Display();
    ~Display();

    static void disp_flush_cb(lv_disp_drv_t* drv, const lv_area_t* area, lv_color_t* color_p);
    static void touch_read_cb(lv_indev_drv_t* drv, lv_indev_data_t* data);

    bool _initialized;
    
    // Display driver
    lv_disp_draw_buf_t _draw_buf;
    lv_disp_drv_t _disp_drv;
    lv_disp_t* _disp;
    
    // Input driver
    lv_indev_drv_t _indev_drv;
    lv_indev_t* _indev;
    
    // Draw buffers (double buffered)
    static lv_color_t _buf1[WIDTH * (HEIGHT / 10)];
    static lv_color_t _buf2[WIDTH * (HEIGHT / 10)];
    
    // Full framebuffer for IPC
    uint16_t _framebuffer[BUFFER_SIZE];
    
    // Touch state
    std::atomic<int16_t> _touch_x;
    std::atomic<int16_t> _touch_y;
    std::atomic<bool> _touch_pressed;
};

/**
 * Initialize LVGL display for host mode
 */
void lvgl_display_init();

/**
 * Initialize LVGL input driver for host mode
 */
void lvgl_input_init();

/**
 * Update LVGL (call in main loop)
 */
void lvgl_update();

} // namespace host

#endif // HOST_DISPLAY_HPP
