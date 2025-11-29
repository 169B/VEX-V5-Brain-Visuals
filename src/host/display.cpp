/**
 * @file display.cpp
 * @brief LVGL Display Driver Implementation for Host Mode
 */

#include "host/display.hpp"
#include "host/ipc.hpp"
#include "liblvgl/lvgl.h"
#include <cstring>
#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <map>
#include <algorithm>
#include <cstdarg>

// LVGL global state (simulated)
static bool lvgl_initialized = false;
static uint32_t lvgl_tick_count = 0;
static auto lvgl_start_time = std::chrono::steady_clock::now();

// Default fonts (stubs)
const lv_font_t lv_font_montserrat_12 = {12, 10};
const lv_font_t lv_font_montserrat_14 = {14, 12};
const lv_font_t lv_font_montserrat_16 = {16, 14};

// Screen object (root)
static lv_obj_t screen_obj = {};
static lv_disp_t display_instance = {};

namespace host {

// Static buffers
lv_color_t Display::_buf1[Display::WIDTH * (Display::HEIGHT / 10)];
lv_color_t Display::_buf2[Display::WIDTH * (Display::HEIGHT / 10)];

// Singleton instance
Display& Display::instance() {
    static Display instance;
    return instance;
}

Display::Display() 
    : _initialized(false), _disp(nullptr), _indev(nullptr),
      _touch_x(0), _touch_y(0), _touch_pressed(false) {
    memset(_framebuffer, 0, sizeof(_framebuffer));
}

Display::~Display() {
    shutdown();
}

void Display::init() {
    if (_initialized) return;
    
    // Initialize LVGL
    lv_init();
    
    // Initialize draw buffer
    lv_disp_draw_buf_init(&_draw_buf, _buf1, _buf2, WIDTH * (HEIGHT / 10));
    
    // Initialize display driver
    lv_disp_drv_init(&_disp_drv);
    _disp_drv.hor_res = WIDTH;
    _disp_drv.ver_res = HEIGHT;
    _disp_drv.flush_cb = disp_flush_cb;
    _disp_drv.draw_buf = &_draw_buf;
    _disp_drv.user_data = this;
    _disp = lv_disp_drv_register(&_disp_drv);
    
    // Initialize input driver
    lv_indev_drv_init(&_indev_drv);
    _indev_drv.type = LV_INDEV_TYPE_POINTER;
    _indev_drv.read_cb = touch_read_cb;
    _indev_drv.user_data = this;
    _indev = lv_indev_drv_register(&_indev_drv);
    
    _initialized = true;
    
    std::cout << "LVGL display initialized (" << WIDTH << "x" << HEIGHT << ")" << std::endl;
}

void Display::shutdown() {
    if (!_initialized) return;
    
    lv_deinit();
    _initialized = false;
    _disp = nullptr;
    _indev = nullptr;
}

bool Display::is_initialized() {
    return _initialized;
}

void Display::set_touch(int16_t x, int16_t y, bool pressed) {
    _touch_x = x;
    _touch_y = y;
    _touch_pressed = pressed;
}

void Display::update() {
    if (!_initialized) return;
    
    // Update LVGL tick
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lvgl_start_time);
    lv_tick_inc(static_cast<uint32_t>(elapsed.count()) - lvgl_tick_count);
    lvgl_tick_count = static_cast<uint32_t>(elapsed.count());
    
    // Handle LVGL tasks
    lv_timer_handler();
}

const uint16_t* Display::get_framebuffer() {
    return _framebuffer;
}

void Display::disp_flush_cb(lv_disp_drv_t* drv, const lv_area_t* area, lv_color_t* color_p) {
    Display* self = static_cast<Display*>(drv->user_data);
    
    // Copy to framebuffer
    for (int32_t y = area->y1; y <= area->y2; y++) {
        for (int32_t x = area->x1; x <= area->x2; x++) {
            if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT) {
                self->_framebuffer[y * WIDTH + x] = color_p->full;
            }
            color_p++;
        }
    }
    
    // Send to IPC if connected
    if (IPCClient::instance().is_connected()) {
        ScreenUpdate update;
        update.x1 = area->x1;
        update.y1 = area->y1;
        update.x2 = area->x2;
        update.y2 = area->y2;
        
        // Copy affected region
        for (int32_t y = area->y1; y <= area->y2; y++) {
            for (int32_t x = area->x1; x <= area->x2; x++) {
                if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT) {
                    update.pixels.push_back(self->_framebuffer[y * WIDTH + x]);
                }
            }
        }
        
        IPCClient::instance().send_screen_update(update);
    }
    
    // Inform LVGL that flushing is complete
    lv_disp_flush_ready(drv);
}

void Display::touch_read_cb(lv_indev_drv_t* drv, lv_indev_data_t* data) {
    Display* self = static_cast<Display*>(drv->user_data);
    
    data->point.x = self->_touch_x;
    data->point.y = self->_touch_y;
    data->state = self->_touch_pressed ? LV_INDEV_STATE_PR : LV_INDEV_STATE_REL;
}

// Convenience functions
void lvgl_display_init() {
    Display::instance().init();
}

void lvgl_input_init() {
    // Input is initialized with display
}

void lvgl_update() {
    Display::instance().update();
}

} // namespace host

/*====================
 * LVGL CORE STUBS
 * Note: These functions have C linkage in LVGL but are implemented in C++.
 * Since we're compiling as C++, we don't need extern "C" for the implementation.
 *====================*/

void lv_init(void) {
    if (lvgl_initialized) return;
    lvgl_initialized = true;
    lvgl_start_time = std::chrono::steady_clock::now();
    lvgl_tick_count = 0;
    
    // Initialize root screen
    memset(&screen_obj, 0, sizeof(screen_obj));
    screen_obj.coords.x1 = 0;
    screen_obj.coords.y1 = 0;
    screen_obj.coords.x2 = 479;
    screen_obj.coords.y2 = 271;
    
    // Initialize display instance
    display_instance.act_scr = &screen_obj;
}

void lv_deinit(void) {
    lvgl_initialized = false;
}

void lv_tick_inc(uint32_t tick_period) {
    lvgl_tick_count += tick_period;
}

uint32_t lv_tick_get(void) {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lvgl_start_time);
    return static_cast<uint32_t>(elapsed.count());
}

void lv_timer_handler(void) {
    // In a full implementation, this would process animations and redraws
    // For the stub, we just ensure callbacks are processed
}

/*====================
 * DISPLAY DRIVER STUBS
 *====================*/

void lv_disp_draw_buf_init(lv_disp_draw_buf_t* draw_buf, void* buf1, void* buf2, uint32_t size_in_px_cnt) {
    draw_buf->buf1 = buf1;
    draw_buf->buf2 = buf2;
    draw_buf->buf_act = buf1;
    draw_buf->size = size_in_px_cnt;
}

void lv_disp_drv_init(lv_disp_drv_t* driver) {
    memset(driver, 0, sizeof(lv_disp_drv_t));
    driver->hor_res = LV_HOR_RES_MAX;
    driver->ver_res = LV_VER_RES_MAX;
    driver->antialiasing = 1;
    driver->dpi = 130;
}

lv_disp_t* lv_disp_drv_register(lv_disp_drv_t* driver) {
    display_instance.driver = driver;
    display_instance.act_scr = &screen_obj;
    return &display_instance;
}

void lv_disp_flush_ready(lv_disp_drv_t* disp_drv) {
    if (disp_drv && disp_drv->draw_buf) {
        disp_drv->draw_buf->flushing = 0;
    }
}

/*====================
 * INPUT DRIVER STUBS
 *====================*/

void lv_indev_drv_init(lv_indev_drv_t* driver) {
    memset(driver, 0, sizeof(lv_indev_drv_t));
    driver->type = LV_INDEV_TYPE_POINTER;
}

static lv_indev_t indev_instance = {};

lv_indev_t* lv_indev_drv_register(lv_indev_drv_t* driver) {
    indev_instance.driver = driver;
    return &indev_instance;
}

/*====================
 * SCREEN FUNCTIONS
 *====================*/

lv_obj_t* lv_scr_act(void) {
    return &screen_obj;
}

void lv_scr_load(lv_obj_t* scr) {
    display_instance.act_scr = scr;
}

void lv_scr_load_anim(lv_obj_t* scr, int anim_type, uint32_t time, uint32_t delay, bool auto_del) {
    (void)anim_type;
    (void)time;
    (void)delay;
    (void)auto_del;
    lv_scr_load(scr);
}

/*====================
 * EVENT FUNCTIONS
 *====================*/

lv_event_code_t lv_event_get_code(lv_event_t* e) {
    return e ? e->code : LV_EVENT_ALL;
}

lv_obj_t* lv_event_get_target(lv_event_t* e) {
    return e ? e->target : nullptr;
}

lv_obj_t* lv_event_get_current_target(lv_event_t* e) {
    return e ? e->current_target : nullptr;
}

void* lv_event_get_user_data(lv_event_t* e) {
    return e ? e->user_data : nullptr;
}

void* lv_event_get_param(lv_event_t* e) {
    return e ? e->param : nullptr;
}

/*====================
 * OBJECT FUNCTIONS
 *====================*/

// Simple object allocator
static std::vector<lv_obj_t*> allocated_objects;

lv_obj_t* lv_obj_create(lv_obj_t* parent) {
    lv_obj_t* obj = new lv_obj_t();
    memset(obj, 0, sizeof(lv_obj_t));
    obj->parent = parent;
    obj->flags = LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE;
    allocated_objects.push_back(obj);
    return obj;
}

void lv_obj_del(lv_obj_t* obj) {
    if (!obj) return;
    auto it = std::find(allocated_objects.begin(), allocated_objects.end(), obj);
    if (it != allocated_objects.end()) {
        allocated_objects.erase(it);
        delete obj;
    }
}

void lv_obj_clean(lv_obj_t* obj) {
    (void)obj;
    // In full implementation, would delete all children
}

void lv_obj_set_pos(lv_obj_t* obj, lv_coord_t x, lv_coord_t y) {
    if (!obj) return;
    lv_coord_t w = obj->coords.x2 - obj->coords.x1;
    lv_coord_t h = obj->coords.y2 - obj->coords.y1;
    obj->coords.x1 = x;
    obj->coords.y1 = y;
    obj->coords.x2 = x + w;
    obj->coords.y2 = y + h;
}

void lv_obj_set_x(lv_obj_t* obj, lv_coord_t x) {
    if (!obj) return;
    lv_coord_t w = obj->coords.x2 - obj->coords.x1;
    obj->coords.x1 = x;
    obj->coords.x2 = x + w;
}

void lv_obj_set_y(lv_obj_t* obj, lv_coord_t y) {
    if (!obj) return;
    lv_coord_t h = obj->coords.y2 - obj->coords.y1;
    obj->coords.y1 = y;
    obj->coords.y2 = y + h;
}

void lv_obj_set_size(lv_obj_t* obj, lv_coord_t w, lv_coord_t h) {
    if (!obj) return;
    obj->coords.x2 = obj->coords.x1 + w;
    obj->coords.y2 = obj->coords.y1 + h;
}

void lv_obj_set_width(lv_obj_t* obj, lv_coord_t w) {
    if (!obj) return;
    obj->coords.x2 = obj->coords.x1 + w;
}

void lv_obj_set_height(lv_obj_t* obj, lv_coord_t h) {
    if (!obj) return;
    obj->coords.y2 = obj->coords.y1 + h;
}

void lv_obj_set_align(lv_obj_t* obj, lv_align_t align) {
    (void)obj;
    (void)align;
    // Would set alignment in full implementation
}

void lv_obj_align(lv_obj_t* obj, lv_align_t align, lv_coord_t x_ofs, lv_coord_t y_ofs) {
    if (!obj || !obj->parent) return;
    
    lv_coord_t pw = obj->parent->coords.x2 - obj->parent->coords.x1;
    lv_coord_t ph = obj->parent->coords.y2 - obj->parent->coords.y1;
    lv_coord_t w = obj->coords.x2 - obj->coords.x1;
    lv_coord_t h = obj->coords.y2 - obj->coords.y1;
    
    lv_coord_t x = 0, y = 0;
    
    switch (align) {
        case LV_ALIGN_TOP_LEFT: x = 0; y = 0; break;
        case LV_ALIGN_TOP_MID: x = (pw - w) / 2; y = 0; break;
        case LV_ALIGN_TOP_RIGHT: x = pw - w; y = 0; break;
        case LV_ALIGN_BOTTOM_LEFT: x = 0; y = ph - h; break;
        case LV_ALIGN_BOTTOM_MID: x = (pw - w) / 2; y = ph - h; break;
        case LV_ALIGN_BOTTOM_RIGHT: x = pw - w; y = ph - h; break;
        case LV_ALIGN_LEFT_MID: x = 0; y = (ph - h) / 2; break;
        case LV_ALIGN_RIGHT_MID: x = pw - w; y = (ph - h) / 2; break;
        case LV_ALIGN_CENTER: x = (pw - w) / 2; y = (ph - h) / 2; break;
        default: break;
    }
    
    lv_obj_set_pos(obj, x + x_ofs, y + y_ofs);
}

void lv_obj_align_to(lv_obj_t* obj, const lv_obj_t* base, lv_align_t align, lv_coord_t x_ofs, lv_coord_t y_ofs) {
    (void)base;
    lv_obj_align(obj, align, x_ofs, y_ofs);
}

void lv_obj_center(lv_obj_t* obj) {
    lv_obj_align(obj, LV_ALIGN_CENTER, 0, 0);
}

lv_coord_t lv_obj_get_x(const lv_obj_t* obj) {
    return obj ? obj->coords.x1 : 0;
}

lv_coord_t lv_obj_get_y(const lv_obj_t* obj) {
    return obj ? obj->coords.y1 : 0;
}

lv_coord_t lv_obj_get_width(const lv_obj_t* obj) {
    return obj ? (obj->coords.x2 - obj->coords.x1) : 0;
}

lv_coord_t lv_obj_get_height(const lv_obj_t* obj) {
    return obj ? (obj->coords.y2 - obj->coords.y1) : 0;
}

void lv_obj_add_flag(lv_obj_t* obj, lv_obj_flag_t f) {
    if (obj) obj->flags |= f;
}

void lv_obj_clear_flag(lv_obj_t* obj, lv_obj_flag_t f) {
    if (obj) obj->flags &= ~f;
}

bool lv_obj_has_flag(const lv_obj_t* obj, lv_obj_flag_t f) {
    return obj ? (obj->flags & f) != 0 : false;
}

void lv_obj_add_state(lv_obj_t* obj, lv_state_t state) {
    if (obj) obj->state |= state;
}

void lv_obj_clear_state(lv_obj_t* obj, lv_state_t state) {
    if (obj) obj->state &= ~state;
}

lv_state_t lv_obj_get_state(const lv_obj_t* obj) {
    return obj ? static_cast<lv_state_t>(obj->state) : LV_STATE_DEFAULT;
}

bool lv_obj_has_state(const lv_obj_t* obj, lv_state_t state) {
    return obj ? (obj->state & state) != 0 : false;
}

// Event handling - simplified
struct event_cb_entry {
    lv_obj_t* obj;
    lv_event_cb_t cb;
    lv_event_code_t filter;
    void* user_data;
};
static std::vector<event_cb_entry> event_callbacks;

void lv_obj_add_event_cb(lv_obj_t* obj, lv_event_cb_t event_cb, lv_event_code_t filter, void* user_data) {
    event_callbacks.push_back({obj, event_cb, filter, user_data});
}

bool lv_obj_remove_event_cb(lv_obj_t* obj, lv_event_cb_t event_cb) {
    auto it = std::find_if(event_callbacks.begin(), event_callbacks.end(),
        [obj, event_cb](const event_cb_entry& e) { return e.obj == obj && e.cb == event_cb; });
    if (it != event_callbacks.end()) {
        event_callbacks.erase(it);
        return true;
    }
    return false;
}

void lv_obj_set_user_data(lv_obj_t* obj, void* user_data) {
    if (obj) obj->user_data = user_data;
}

void* lv_obj_get_user_data(const lv_obj_t* obj) {
    return obj ? obj->user_data : nullptr;
}

lv_obj_t* lv_obj_get_parent(const lv_obj_t* obj) {
    return obj ? obj->parent : nullptr;
}

lv_obj_t* lv_obj_get_child(const lv_obj_t* obj, int32_t id) {
    (void)obj;
    (void)id;
    return nullptr; // Simplified - no child tracking
}

uint32_t lv_obj_get_child_cnt(const lv_obj_t* obj) {
    (void)obj;
    return 0; // Simplified
}

/*====================
 * STYLE FUNCTIONS
 *====================*/

void lv_style_init(lv_style_t* style) {
    if (style) memset(style, 0, sizeof(lv_style_t));
}

void lv_style_reset(lv_style_t* style) {
    lv_style_init(style);
}

void lv_style_set_width(lv_style_t* style, lv_coord_t value) { (void)style; (void)value; }
void lv_style_set_height(lv_style_t* style, lv_coord_t value) { (void)style; (void)value; }
void lv_style_set_bg_color(lv_style_t* style, lv_color_t color) { (void)style; (void)color; }
void lv_style_set_bg_opa(lv_style_t* style, lv_opa_t value) { (void)style; (void)value; }
void lv_style_set_text_color(lv_style_t* style, lv_color_t color) { (void)style; (void)color; }
void lv_style_set_text_font(lv_style_t* style, const lv_font_t* font) { (void)style; (void)font; }
void lv_style_set_border_width(lv_style_t* style, lv_coord_t value) { (void)style; (void)value; }
void lv_style_set_border_color(lv_style_t* style, lv_color_t color) { (void)style; (void)color; }
void lv_style_set_radius(lv_style_t* style, lv_coord_t value) { (void)style; (void)value; }
void lv_style_set_pad_all(lv_style_t* style, lv_coord_t value) { (void)style; (void)value; }
void lv_style_set_pad_top(lv_style_t* style, lv_coord_t value) { (void)style; (void)value; }
void lv_style_set_pad_bottom(lv_style_t* style, lv_coord_t value) { (void)style; (void)value; }
void lv_style_set_pad_left(lv_style_t* style, lv_coord_t value) { (void)style; (void)value; }
void lv_style_set_pad_right(lv_style_t* style, lv_coord_t value) { (void)style; (void)value; }

void lv_obj_add_style(lv_obj_t* obj, lv_style_t* style, lv_style_selector_t selector) { (void)obj; (void)style; (void)selector; }
void lv_obj_remove_style(lv_obj_t* obj, lv_style_t* style, lv_style_selector_t selector) { (void)obj; (void)style; (void)selector; }
void lv_obj_remove_style_all(lv_obj_t* obj) { (void)obj; }

void lv_obj_set_style_bg_color(lv_obj_t* obj, lv_color_t color, lv_style_selector_t selector) { (void)obj; (void)color; (void)selector; }
void lv_obj_set_style_bg_opa(lv_obj_t* obj, lv_opa_t value, lv_style_selector_t selector) { (void)obj; (void)value; (void)selector; }
void lv_obj_set_style_text_color(lv_obj_t* obj, lv_color_t color, lv_style_selector_t selector) { (void)obj; (void)color; (void)selector; }
void lv_obj_set_style_border_width(lv_obj_t* obj, lv_coord_t value, lv_style_selector_t selector) { (void)obj; (void)value; (void)selector; }
void lv_obj_set_style_border_color(lv_obj_t* obj, lv_color_t color, lv_style_selector_t selector) { (void)obj; (void)color; (void)selector; }
void lv_obj_set_style_radius(lv_obj_t* obj, lv_coord_t value, lv_style_selector_t selector) { (void)obj; (void)value; (void)selector; }
void lv_obj_set_style_pad_all(lv_obj_t* obj, lv_coord_t value, lv_style_selector_t selector) { (void)obj; (void)value; (void)selector; }
void lv_obj_set_style_pad_top(lv_obj_t* obj, lv_coord_t value, lv_style_selector_t selector) { (void)obj; (void)value; (void)selector; }
void lv_obj_set_style_pad_bottom(lv_obj_t* obj, lv_coord_t value, lv_style_selector_t selector) { (void)obj; (void)value; (void)selector; }
void lv_obj_set_style_pad_left(lv_obj_t* obj, lv_coord_t value, lv_style_selector_t selector) { (void)obj; (void)value; (void)selector; }
void lv_obj_set_style_pad_right(lv_obj_t* obj, lv_coord_t value, lv_style_selector_t selector) { (void)obj; (void)value; (void)selector; }

/*====================
 * WIDGET STUBS
 *====================*/

lv_obj_t* lv_btn_create(lv_obj_t* parent) {
    lv_obj_t* btn = lv_obj_create(parent);
    btn->flags |= LV_OBJ_FLAG_CLICKABLE;
    lv_obj_set_size(btn, 100, 40);
    return btn;
}

// Label storage (simplified)
static std::map<lv_obj_t*, std::string> label_texts;

lv_obj_t* lv_label_create(lv_obj_t* parent) {
    lv_obj_t* label = lv_obj_create(parent);
    label->flags &= ~LV_OBJ_FLAG_CLICKABLE;
    label_texts[label] = "";
    return label;
}

void lv_label_set_text(lv_obj_t* obj, const char* txt) {
    if (obj && txt) label_texts[obj] = txt;
}

void lv_label_set_text_fmt(lv_obj_t* obj, const char* fmt, ...) {
    if (!obj || !fmt) return;
    char buffer[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    label_texts[obj] = buffer;
}

void lv_label_set_text_static(lv_obj_t* obj, const char* txt) {
    lv_label_set_text(obj, txt);
}

void lv_label_set_long_mode(lv_obj_t* obj, lv_label_long_mode_t mode) { (void)obj; (void)mode; }
void lv_label_set_recolor(lv_obj_t* obj, bool en) { (void)obj; (void)en; }

const char* lv_label_get_text(const lv_obj_t* obj) {
    if (!obj) return "";
    auto it = label_texts.find(const_cast<lv_obj_t*>(obj));
    if (it != label_texts.end()) return it->second.c_str();
    return "";
}

lv_label_long_mode_t lv_label_get_long_mode(const lv_obj_t* obj) {
    (void)obj;
    return LV_LABEL_LONG_WRAP;
}

// Tabview
struct tabview_data {
    std::vector<lv_obj_t*> tabs;
    uint16_t active;
};
static std::map<lv_obj_t*, tabview_data> tabview_map;

lv_obj_t* lv_tabview_create(lv_obj_t* parent, lv_dir_t tab_pos, lv_coord_t tab_size) {
    (void)tab_pos;
    (void)tab_size;
    lv_obj_t* tv = lv_obj_create(parent);
    tabview_map[tv] = {{}, 0};
    lv_obj_set_size(tv, 480, 272);
    return tv;
}

lv_obj_t* lv_tabview_add_tab(lv_obj_t* tv, const char* name) {
    if (!tv) return nullptr;
    lv_obj_t* tab = lv_obj_create(tv);
    lv_obj_set_size(tab, 480, 222); // Below tab buttons
    label_texts[tab] = name ? name : "";
    tabview_map[tv].tabs.push_back(tab);
    return tab;
}

void lv_tabview_set_act(lv_obj_t* tv, uint32_t id, int anim_type) {
    (void)anim_type;
    if (tv) tabview_map[tv].active = id;
}

uint16_t lv_tabview_get_tab_act(lv_obj_t* tv) {
    if (!tv) return 0;
    return tabview_map[tv].active;
}

lv_obj_t* lv_tabview_get_content(lv_obj_t* tv) {
    return tv;
}

lv_obj_t* lv_tabview_get_tab_btns(lv_obj_t* tv) {
    return tv;
}

// Button matrix
static std::map<lv_obj_t*, std::vector<std::string>> btnm_maps;
static std::map<lv_obj_t*, uint16_t> btnm_selected;

lv_obj_t* lv_btnmatrix_create(lv_obj_t* parent) {
    lv_obj_t* btnm = lv_obj_create(parent);
    btnm_maps[btnm] = {};
    btnm_selected[btnm] = 0;
    return btnm;
}

void lv_btnmatrix_set_map(lv_obj_t* obj, const char* map[]) {
    if (!obj || !map) return;
    btnm_maps[obj].clear();
    for (int i = 0; map[i] && map[i][0] != '\0'; i++) {
        btnm_maps[obj].push_back(map[i]);
    }
}

void lv_btnmatrix_set_ctrl_map(lv_obj_t* obj, const lv_btnmatrix_ctrl_t ctrl_map[]) { (void)obj; (void)ctrl_map; }
void lv_btnmatrix_set_btn_ctrl(lv_obj_t* obj, uint16_t btn_id, lv_btnmatrix_ctrl_t ctrl) { (void)obj; (void)btn_id; (void)ctrl; }
void lv_btnmatrix_clear_btn_ctrl(lv_obj_t* obj, uint16_t btn_id, lv_btnmatrix_ctrl_t ctrl) { (void)obj; (void)btn_id; (void)ctrl; }
void lv_btnmatrix_set_btn_ctrl_all(lv_obj_t* obj, lv_btnmatrix_ctrl_t ctrl) { (void)obj; (void)ctrl; }
void lv_btnmatrix_clear_btn_ctrl_all(lv_obj_t* obj, lv_btnmatrix_ctrl_t ctrl) { (void)obj; (void)ctrl; }
void lv_btnmatrix_set_one_checked(lv_obj_t* obj, bool en) { (void)obj; (void)en; }

uint16_t lv_btnmatrix_get_selected_btn(const lv_obj_t* obj) {
    if (!obj) return 0;
    auto it = btnm_selected.find(const_cast<lv_obj_t*>(obj));
    return it != btnm_selected.end() ? it->second : 0;
}

const char* lv_btnmatrix_get_btn_text(const lv_obj_t* obj, uint16_t btn_id) {
    if (!obj) return "";
    auto it = btnm_maps.find(const_cast<lv_obj_t*>(obj));
    if (it != btnm_maps.end() && btn_id < it->second.size()) {
        return it->second[btn_id].c_str();
    }
    return "";
}

bool lv_btnmatrix_has_btn_ctrl(const lv_obj_t* obj, uint16_t btn_id, lv_btnmatrix_ctrl_t ctrl) {
    (void)obj; (void)btn_id; (void)ctrl;
    return false;
}

// Other widgets - minimal stubs
lv_obj_t* lv_bar_create(lv_obj_t* parent) { return lv_obj_create(parent); }
void lv_bar_set_value(lv_obj_t* obj, int32_t value, int anim) { (void)obj; (void)value; (void)anim; }
void lv_bar_set_range(lv_obj_t* obj, int32_t min, int32_t max) { (void)obj; (void)min; (void)max; }
int32_t lv_bar_get_value(const lv_obj_t* obj) { (void)obj; return 0; }
int32_t lv_bar_get_min_value(const lv_obj_t* obj) { (void)obj; return 0; }
int32_t lv_bar_get_max_value(const lv_obj_t* obj) { (void)obj; return 100; }

lv_obj_t* lv_slider_create(lv_obj_t* parent) { return lv_obj_create(parent); }
void lv_slider_set_value(lv_obj_t* obj, int32_t value, int anim) { (void)obj; (void)value; (void)anim; }
void lv_slider_set_range(lv_obj_t* obj, int32_t min, int32_t max) { (void)obj; (void)min; (void)max; }
int32_t lv_slider_get_value(const lv_obj_t* obj) { (void)obj; return 0; }
int32_t lv_slider_get_min_value(const lv_obj_t* obj) { (void)obj; return 0; }
int32_t lv_slider_get_max_value(const lv_obj_t* obj) { (void)obj; return 100; }

lv_obj_t* lv_switch_create(lv_obj_t* parent) { return lv_obj_create(parent); }
lv_obj_t* lv_checkbox_create(lv_obj_t* parent) { return lv_obj_create(parent); }
void lv_checkbox_set_text(lv_obj_t* obj, const char* txt) { lv_label_set_text(obj, txt); }
void lv_checkbox_set_text_static(lv_obj_t* obj, const char* txt) { lv_label_set_text(obj, txt); }
const char* lv_checkbox_get_text(const lv_obj_t* obj) { return lv_label_get_text(obj); }

lv_obj_t* lv_dropdown_create(lv_obj_t* parent) { return lv_obj_create(parent); }
void lv_dropdown_set_options(lv_obj_t* obj, const char* options) { (void)obj; (void)options; }
void lv_dropdown_set_selected(lv_obj_t* obj, uint16_t sel_opt) { (void)obj; (void)sel_opt; }
uint16_t lv_dropdown_get_selected(const lv_obj_t* obj) { (void)obj; return 0; }
void lv_dropdown_get_selected_str(const lv_obj_t* obj, char* buf, uint32_t buf_size) { (void)obj; if (buf && buf_size) buf[0] = 0; }

lv_obj_t* lv_roller_create(lv_obj_t* parent) { return lv_obj_create(parent); }
void lv_roller_set_options(lv_obj_t* obj, const char* options, lv_roller_mode_t mode) { (void)obj; (void)options; (void)mode; }
void lv_roller_set_selected(lv_obj_t* obj, uint16_t sel_opt, int anim) { (void)obj; (void)sel_opt; (void)anim; }
uint16_t lv_roller_get_selected(const lv_obj_t* obj) { (void)obj; return 0; }
void lv_roller_get_selected_str(const lv_obj_t* obj, char* buf, uint32_t buf_size) { (void)obj; if (buf && buf_size) buf[0] = 0; }

lv_obj_t* lv_textarea_create(lv_obj_t* parent) { return lv_obj_create(parent); }
void lv_textarea_add_char(lv_obj_t* obj, uint32_t c) { (void)obj; (void)c; }
void lv_textarea_add_text(lv_obj_t* obj, const char* txt) { (void)obj; (void)txt; }
void lv_textarea_del_char(lv_obj_t* obj) { (void)obj; }
void lv_textarea_del_char_forward(lv_obj_t* obj) { (void)obj; }
void lv_textarea_set_text(lv_obj_t* obj, const char* txt) { lv_label_set_text(obj, txt); }
void lv_textarea_set_placeholder_text(lv_obj_t* obj, const char* txt) { (void)obj; (void)txt; }
const char* lv_textarea_get_text(const lv_obj_t* obj) { return lv_label_get_text(obj); }

lv_obj_t* lv_table_create(lv_obj_t* parent) { return lv_obj_create(parent); }
void lv_table_set_cell_value(lv_obj_t* obj, uint16_t row, uint16_t col, const char* txt) { (void)obj; (void)row; (void)col; (void)txt; }
void lv_table_set_row_cnt(lv_obj_t* obj, uint16_t row_cnt) { (void)obj; (void)row_cnt; }
void lv_table_set_col_cnt(lv_obj_t* obj, uint16_t col_cnt) { (void)obj; (void)col_cnt; }
void lv_table_set_col_width(lv_obj_t* obj, uint16_t col_id, lv_coord_t w) { (void)obj; (void)col_id; (void)w; }
uint16_t lv_table_get_row_cnt(const lv_obj_t* obj) { (void)obj; return 0; }
uint16_t lv_table_get_col_cnt(const lv_obj_t* obj) { (void)obj; return 0; }
const char* lv_table_get_cell_value(const lv_obj_t* obj, uint16_t row, uint16_t col) { (void)obj; (void)row; (void)col; return ""; }

lv_obj_t* lv_list_create(lv_obj_t* parent) { return lv_obj_create(parent); }
lv_obj_t* lv_list_add_text(lv_obj_t* list, const char* txt) { lv_obj_t* o = lv_obj_create(list); lv_label_set_text(o, txt); return o; }
lv_obj_t* lv_list_add_btn(lv_obj_t* list, const void* icon, const char* txt) { (void)icon; return lv_list_add_text(list, txt); }
const char* lv_list_get_btn_text(const lv_obj_t* list, const lv_obj_t* btn) { (void)list; return lv_label_get_text(btn); }

lv_obj_t* lv_msgbox_create(lv_obj_t* parent, const char* title, const char* txt, const char* btn_txts[], bool add_close_btn) {
    (void)title; (void)txt; (void)btn_txts; (void)add_close_btn;
    return lv_obj_create(parent);
}
lv_obj_t* lv_msgbox_get_btns(lv_obj_t* mbox) { return mbox; }
const char* lv_msgbox_get_active_btn_text(lv_obj_t* mbox) { (void)mbox; return ""; }
void lv_msgbox_close(lv_obj_t* mbox) { lv_obj_del(mbox); }

lv_obj_t* lv_spinner_create(lv_obj_t* parent, uint32_t time, uint32_t arc_length) { (void)time; (void)arc_length; return lv_obj_create(parent); }

lv_obj_t* lv_chart_create(lv_obj_t* parent) { return lv_obj_create(parent); }
void lv_chart_set_type(lv_obj_t* obj, lv_chart_type_t type) { (void)obj; (void)type; }
void lv_chart_set_point_count(lv_obj_t* obj, uint16_t cnt) { (void)obj; (void)cnt; }
void lv_chart_set_range(lv_obj_t* obj, lv_chart_axis_t axis, lv_coord_t min, lv_coord_t max) { (void)obj; (void)axis; (void)min; (void)max; }
void lv_chart_set_update_mode(lv_obj_t* obj, lv_chart_update_mode_t update_mode) { (void)obj; (void)update_mode; }
lv_chart_series_t* lv_chart_add_series(lv_obj_t* obj, lv_color_t color, lv_chart_axis_t axis) { (void)obj; (void)color; (void)axis; return nullptr; }
void lv_chart_remove_series(lv_obj_t* obj, lv_chart_series_t* series) { (void)obj; (void)series; }
void lv_chart_set_next_value(lv_obj_t* obj, lv_chart_series_t* ser, lv_coord_t value) { (void)obj; (void)ser; (void)value; }
void lv_chart_refresh(lv_obj_t* obj) { (void)obj; }

lv_obj_t* lv_led_create(lv_obj_t* parent) { return lv_obj_create(parent); }
void lv_led_set_color(lv_obj_t* obj, lv_color_t color) { (void)obj; (void)color; }
void lv_led_set_brightness(lv_obj_t* obj, uint8_t bright) { (void)obj; (void)bright; }
void lv_led_on(lv_obj_t* obj) { (void)obj; }
void lv_led_off(lv_obj_t* obj) { (void)obj; }
void lv_led_toggle(lv_obj_t* obj) { (void)obj; }
uint8_t lv_led_get_brightness(const lv_obj_t* obj) { (void)obj; return 255; }
